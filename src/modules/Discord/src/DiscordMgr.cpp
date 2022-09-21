#include <dpp/dpp.h>
#include <dpp/presence.h>

#include "Database/DatabaseEnv.h"
#include "Globals/SharedDefines.h"
#include "Entities/Player.h"
#include "Entities/ItemPrototype.h"
#include "Server/DBCStores.h"
#include "Server/DBCStructure.h"
#include "Policies/Singleton.h"

#include "DiscordMgr.h"

INSTANTIATE_SINGLETON_1(DiscordMgr);

const std::string BOT_TOKEN = "BOT TOKEN"; // generate token through discord dev portal (create your bot)
const dpp::snowflake CHANNEL_ID = 0; // some channel to announce into (put discord in dev mode and grab channel ID)

DiscordMgr::DiscordMgr() 
{
}

DiscordMgr::~DiscordMgr()
{
}

void DiscordMgr::Init()
{         
    dpp::cluster bot(BOT_TOKEN); 
    bot.on_log(dpp::utility::cout_logger());

    bot.on_slashcommand([](const dpp::slashcommand_t& event) 
    {
        if (event.command.get_command_name() == "status") 
        {
            dpp::embed embed = dpp::embed().
            set_color(dpp::colors::green).
            set_title("Server is online!");
            event.reply(dpp::message().add_embed(embed)); 
        }
        else if (event.command.get_command_name() == "who") 
        {
            QueryResult* result = CharacterDatabase.Query("SELECT name,race FROM characters WHERE online = '1'");
            if (!result) {
                dpp::embed embed = dpp::embed().
                set_color(dpp::colors::red).
                set_title("No players online!");
                event.reply(dpp::message().add_embed(embed)); 
            }
            else {
                std::string hordeText = "(none)";
                std::string allianceText = "(none)";
                do
                {
                    Field* fields = result->Fetch();

                    std::string name = fields[0].GetCppString(); // name
                    uint8 race = fields[1].GetUInt8(); // race

                    if (race == RACE_HUMAN || race == RACE_DWARF || race == RACE_NIGHTELF || 
                        race == RACE_GNOME || race == RACE_DRAENEI)
                    {
                        if (allianceText.compare("(none)") == 0)
                        {
                            allianceText = name + "\n";
                        }
                        else
                        {
                            allianceText += name + "\n";
                        }
                    }
                    else 
                    {
                        if (hordeText.compare("(none)") == 0)
                        {
                            hordeText = name + "\n";
                        }
                        else
                        {
                            hordeText += name + "\n";
                        }
                    }
                }
                while (result->NextRow());
                delete result;

                dpp::embed embed = dpp::embed().
                set_color(dpp::colors::green).
                set_title("Players online").
                add_field(
                    "Horde",
                    hordeText,
                    true
                ).
                add_field(
                    "Alliance",
                    allianceText,
                    true
                );
                event.reply(dpp::message().add_embed(embed)); 
            }      
        }
        else if (event.command.get_command_name() == "inspect")
        {
            std::string characterName = std::get<std::string>(event.get_parameter("character"));

            CharacterDatabase.escape_string(characterName);
            QueryResult* result = CharacterDatabase.PQuery("SELECT name,race,class,gender,level,online,guid FROM characters WHERE name = '%s'", characterName.c_str());
            if (!result) 
            {
                dpp::embed embed = dpp::embed().
                set_color(dpp::colors::red).
                set_title("Character not found!");
                event.reply(dpp::message().add_embed(embed)); 
            } 
            else 
            {
                bool _horde = true;
                Field* fields = result->Fetch();

                std::string name = fields[0].GetCppString(); // name
                uint8 race = fields[1].GetUInt8(); // race
                uint8 classId = fields[2].GetUInt8(); // class
                uint8 gender = fields[3].GetUInt8(); // gender
                uint8 level = fields[4].GetUInt8(); // level
                uint8 online = fields[5].GetUInt8(); // level
                uint8 guid = fields[6].GetUInt8(); // player id

                if (race == RACE_HUMAN || race == RACE_DWARF || race == RACE_NIGHTELF || 
                    race == RACE_GNOME || race == RACE_DRAENEI)
                {
                    _horde = false;
                }

                std::string genderStr = std::to_string(gender);
                std::string raceStr = std::to_string(race);
                std::string classStr = std::to_string(classId);
                std::string levelStr = std::to_string(level);
                std::string guildName = "(none)";
                std::string guildNameUri = "";
                
                // check if guild?
                bool hasGuild = false;
                QueryResult* guildResult = CharacterDatabase.PQuery("SELECT guildid FROM guild_member WHERE guid = '%u'", guid);
                if (guildResult)
                {
                    Field* gfields = guildResult->Fetch();
                    uint8 guildId = gfields[0].GetUInt8(); // guild ID

                    // get guild name
                    guildResult = CharacterDatabase.PQuery("SELECT name FROM guild WHERE guildid = '%u'", guildId);
                    gfields = guildResult->Fetch();
                    guildName = gfields[0].GetCppString(); // guild Name

                    /*
                     * if you have armory installed or some other database, edit here

                    // replace spaces with +
                    std::string guildNameFixed = guildName; // guild Name
                    size_t start_pos = 0;
                    while((start_pos = guildNameFixed.find(std::string(" "), start_pos)) != std::string::npos) {
                        guildNameFixed.replace(start_pos, std::string(" ").length(), std::string("+"));
                        start_pos += std::string("+").length(); // Handles case where 'to' is a substring of 'from'
                    }
                    guildNameUri = "https://example.com/armory/guild-info.xml?gn="+guildNameFixed+"&r=RealmNameHere";

                    *
                    */
                    hasGuild = true;
                    delete guildResult;
                } 

                dpp::embed embed = dpp::embed().
                set_title(name).
                set_description("Level "+levelStr+", " + sDiscordMgr.GetRaceName(race) + " "+ sDiscordMgr.GetClassName(classId)).
                //add_field("Guild", hasGuild == true ? "["+guildName+"]("+guildNameUri+")" : guildName).
                add_field("Guild", guildName).
                set_color(_horde ? dpp::colors::scarlet_red : dpp::colors::sti_blue).
                set_footer(dpp::embed_footer().set_text(online == 1 ? "Online" : "Offline"));
                event.reply(dpp::message().add_embed(embed)); 

                delete result;
            }
        }
    });
    
    bot.on_ready([&bot](const dpp::ready_t& event) {
        if (dpp::run_once<struct register_bot_commands>()) {
            // register cmd
            bot.global_command_create(dpp::slashcommand("status", "Shows server status.", bot.me.id));
            bot.global_command_create(dpp::slashcommand("who", "Shows who is online.", bot.me.id));

            // with sub cmd
            dpp::slashcommand inspect("inspect", "Inspect a character.", bot.me.id);
            inspect.add_option(
                dpp::command_option(dpp::co_string, "character", "Character name to inspect.", true)
            );
            bot.global_command_create(inspect);

            std::string presence = "Azeroth";
            bot.set_presence(dpp::presence(dpp::ps_dnd, dpp::at_watching, presence));
        }
    });
    bot.start(false);
}

// helpers
std::string DiscordMgr::GetRaceName(uint8 race)
{
    switch(race)
    {
        case 1: // human
            return "Human";
        case 2: 
            return "Orc";
        case 3:
            return "Dwarf";
        case 4:
            return "Night Elf";
        case 5:
            return "Undead";
        case 6:
            return "Tauren";
        case 7:
            return "Gnome";
        case 8:
            return "Troll";
        case 9:
            return "Goblin";
        case 10:
            return "Blood Elf";
        case 11:
            return "Draenei";
    }
    return "Unknown";
}

std::string DiscordMgr::GetClassName(uint8 classId)
{
    switch(classId)
    {
        case 1:
            return "Warrior";
        case 2:
            return "Paladin";
        case 3:
            return "Hunter";
        case 4:
            return "Rogue";
        case 5:
            return "Priest";
        case 6:
            return "Death Knight";
        case 7:
            return "Shaman";
        case 8:
            return "Mage";
        case 9:
            return "Warlock";
        case 11:
            return "Druid";
    }
    return "Unknown";
}

// server exit
void DiscordMgr::Exit()
{ 
    dpp::cluster bot(BOT_TOKEN); 
    bot.message_create(dpp::message(CHANNEL_ID, ":red_circle: Server went offline!"));
}

// achievements
void DiscordMgr::BroadcastAchievement(Player* player, AchievementEntry const* achievement)
{
    dpp::cluster bot(BOT_TOKEN); 

    std::string pName = player->GetName();
    std::string aName = achievement->name[0];
    std::string aID = std::to_string(achievement->ID);

    dpp::embed embed = dpp::embed().
        set_color(dpp::colors::dark_goldenrod).
        set_title(":trophy: "+aName).
        set_url("https://wotlkdb.com/?achievement="+aID).
        set_description("Earned by " + pName);

    bot.message_create(dpp::message(CHANNEL_ID, embed));
}

// items
void DiscordMgr::BroadcastEpicItem(Player* player, ItemPrototype const* item)
{
    dpp::cluster bot(BOT_TOKEN); 

    std::string pName = player->GetName();
    std::string iName = item->Name1;
    std::string iID = std::to_string(item->ItemId);

    std::string icon = ":purple_circle:";
    if (item->Class == ITEM_CLASS_WEAPON)
        icon = ":dagger:";
    else if (item->Class == ITEM_CLASS_ARMOR)
        icon = ":shield:";

    // TODO: check quality colors

    dpp::embed embed = dpp::embed().
        set_color(dpp::colors::vivid_violet). 
        set_title(icon + " " + iName).
        set_url("https://wotlkdb.com/?item="+iID).
        set_description("Looted by " + pName);

    bot.message_create(dpp::message(CHANNEL_ID, embed));
}
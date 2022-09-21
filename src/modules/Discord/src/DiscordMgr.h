#include <dpp/dpp.h>

#include "Database/DatabaseEnv.h"
#include "Entities/Player.h"
#include "Entities/ItemPrototype.h"
#include "Server/DBCStructure.h"

class DiscordMgr
{
    public:
        DiscordMgr();
        ~DiscordMgr();

        void Init();
        void Exit();
        void BroadcastAchievement(Player* player, AchievementEntry const* achievement);
        void BroadcastEpicItem(Player* player, ItemPrototype const* item);

        std::string GetRaceName(uint8 race);
        std::string GetClassName(uint8 classId);
};

#define sDiscordMgr MaNGOS::Singleton<DiscordMgr>::Instance()
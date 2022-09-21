#include "Common.h"
#include "Threading.h"

/// Heartbeat thread for the World
class DiscordThread : public MaNGOS::Runnable
{
    public:
        void run() override;
};
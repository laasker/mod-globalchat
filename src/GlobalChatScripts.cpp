/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ChannelMgr.h"
#include "GlobalChatMgr.h"
#include "ScriptMgr.h"

class GlobalChat_Config : public WorldScript
{
public:
    GlobalChat_Config() : WorldScript("GlobalChat_Config", { WORLDHOOK_ON_AFTER_CONFIG_LOAD }) {}

    void OnAfterConfigLoad(bool reload) override
    {
        sGlobalChatMgr->LoadConfig(reload);
    }
};

class GlobalChat_Player : public PlayerScript
{
public:
    GlobalChat_Player() : PlayerScript("GlobalChat_Player", { PLAYERHOOK_ON_LOGIN, PLAYERHOOK_ON_SAVE, PLAYERHOOK_CAN_PLAYER_USE_CHAT }) {} // PLAYERHOOK_ON_BEFORE_SEND_CHAT_MESSAGE

    void OnPlayerLogin(Player* player) override
    {
        if (sGlobalChatMgr->GlobalChatEnabled)
        {
            if (sGlobalChatMgr->Announce)
            {
                ChatHandler(player->GetSession()).PSendSysMessage("This server is running the |cff4CFF00GlobalChat|r module. Use |cff4CFF00.help global|r to find out how to use it.");
            }

            sGlobalChatMgr->LoadPlayerData(player);

            if (!sGlobalChatMgr->IsInChat(player->GetGUID()))
            {
                if (sGlobalChatMgr->JoinChannel && !sGlobalChatMgr->ChatName.empty())
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("You can join the |cffFF0000GlobalChat|r by typing |cffFF0000.joinglobal|r or |cffFF0000/join {}|r at any time.", sGlobalChatMgr->ChatName.c_str());
                }
                else
                {
                    ChatHandler(player->GetSession()).PSendSysMessage("You can join the |cffFF0000GlobalChat|r by typing |cffFF0000.joinglobal|r at any time.");
                }
            }
        }
    }

    void OnPlayerSave(Player* player) override
    {
        sGlobalChatMgr->SavePlayerData(player);
    }

    [[nodiscard]] bool OnPlayerCanUseChat(Player* player, uint32 /*type*/, uint32 language, std::string& msg, Channel* channel) override
    {
        if (!channel || !sGlobalChatMgr->JoinChannel || sGlobalChatMgr->ChatName.empty() || language == LANG_ADDON)
            return true;

        if (channel->GetName() != sGlobalChatMgr->ChatName)
            return true;

        if (sGlobalChatMgr->FactionSpecific && player->GetSession()->GetSecurity() > 0)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("Please use |cff4CFF00.galliance|r or |cff4CFF00.ghorde|r for the GlobalChat as GM.");
            return false;
        }

        sGlobalChatMgr->SendGlobalChat(player->GetSession(), msg.c_str());
        return false;
    }
};

void AddSC_GlobalChat()
{
    new GlobalChat_Config();
    new GlobalChat_Player();
}

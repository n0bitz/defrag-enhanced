#include "qagame.h"

/*
================
ClientEvents

Events will be passed on to the clients for presentation,
but any server game effects are handled here
================
*/
DEFINE_HOOK(void, ClientEvents, (gentity_t* ent, int oldEventSequence))
    int i, j;
    int event;
    gclient_t* client;
    int damage;
    vec3_t dir;
    vec3_t origin, angles;
    //	qboolean	fired;
    gitem_t* item;
    // gentity_t* drop;

    (void)ORIGINAL(ClientEvents);  // intentional reimplementation

    client = ent->client;

    if (oldEventSequence < client->ps.eventSequence - MAX_PS_EVENTS) {
        oldEventSequence = client->ps.eventSequence - MAX_PS_EVENTS;
    }
    for (i = oldEventSequence; i < client->ps.eventSequence; i++) {
        event = client->ps.events[i & (MAX_PS_EVENTS - 1)];

        switch (event) {
            case EV_FALL_MEDIUM:
            case EV_FALL_FAR:
                if (ent->s.eType != ET_PLAYER) {
                    break;  // not in the player model
                }
                if (event == EV_FALL_FAR) {
                    damage = 10;
                } else {
                    damage = 5;
                }
                VectorSet(dir, 0, 0, 1);
                ent->pain_debounce_time =
                   level.time + 200;  // no normal pain sound
                G_Damage(ent, NULL, NULL, NULL, NULL, damage, 0, MOD_FALLING);
                break;

            case EV_FIRE_WEAPON:
                FireWeapon(ent);
                break;

            case EV_USE_ITEM1:  // teleporter
                // remove flags in CTF
                item = NULL;
                j = 0;

                if (ent->client->ps.powerups[PW_REDFLAG]) {
                    item = BG_FindItemForPowerup(PW_REDFLAG);
                    j = PW_REDFLAG;
                } else if (ent->client->ps.powerups[PW_BLUEFLAG]) {
                    item = BG_FindItemForPowerup(PW_BLUEFLAG);
                    j = PW_BLUEFLAG;
                } else if (ent->client->ps.powerups[PW_NEUTRALFLAG]) {
                    item = BG_FindItemForPowerup(PW_NEUTRALFLAG);
                    j = PW_NEUTRALFLAG;
                }

                if (item) {
                    // DFE change: DeFRaG kept the original baseq3 behaviour of
                    // dropping the flags and instead chose to never let the
                    // player be able to hold a personal teleporter and thus not
                    // be able to use it (normally) at all. However, we now let
                    // the player be able to pick up the personal teleporter.
                    // So, to prevent new unintended advantageous time resets
                    // off the dropped flag, we simply don't drop the flag at
                    // all and just keep it removed from the player like before.
                    /*
                    drop = Drop_Item(ent, item, 0);
                    // decide how many seconds it has left
                    drop->count =
                       (ent->client->ps.powerups[j] - level.time) / 1000;
                    if (drop->count < 1) {
                        drop->count = 1;
                    }
                    */

                    ent->client->ps.powerups[j] = 0;
                }

                SelectRandomFurthestSpawnPoint(ent->client->ps.origin, origin,
                                               angles, ent->client->arena);
                TeleportPlayer(ent, origin, angles);
                break;

            case EV_USE_ITEM2:  // medkit
                ent->health = ent->client->ps.stats[STAT_MAX_HEALTH] + 25;
                break;

            default:
                break;
        }
    }
END_HOOK

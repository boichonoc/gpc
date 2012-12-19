/*****************************************************************************
* Model: game.qm
* File:  ././ship.c
*
* This file has been generated automatically by QP Modeler (QM).
* DO NOT EDIT THIS FILE MANUALLY.
*
* Please visit www.state-machine.com/qm for more information.
*****************************************************************************/
#include "qp_port.h"
#include "bsp.h"
#include "game.h"

/* Q_DEFINE_THIS_FILE */

#define SHIP_WIDTH  5
#define SHIP_HEIGHT 3

/* encapsulated delcaration of the Ship active object ----------------------*/
/* @(/2/1) .................................................................*/
/** 
* Ship Active Object
*/
typedef struct ShipTag {
/* protected: */
    QActive super;

/* private: */
    uint8_t x;
    uint8_t y;
    uint8_t exp_ctr;
    uint16_t score;
} Ship;

/* protected: */
static QState Ship_initial(Ship * const me, QEvt const * const e);
static QState Ship_active(Ship * const me, QEvt const * const e);
static QState Ship_parked(Ship * const me, QEvt const * const e);
static QState Ship_flying(Ship * const me, QEvt const * const e);
static QState Ship_exploding(Ship * const me, QEvt const * const e);


/* local objects -----------------------------------------------------------*/
static Ship l_ship; /* the sole instance of the Ship active object */

/* Public-scope objects ----------------------------------------------------*/
QActive * const AO_Ship = (QActive *)&l_ship; /* opaque pointer */

/* Active object definition ------------------------------------------------*/
/* @(/2/10) ................................................................*/
void Ship_ctor(void) {
    Ship *me = &l_ship;
    QActive_ctor(&me->super, (QStateHandler)&Ship_initial);
    me->x = GAME_SHIP_X;
    me->y = GAME_SHIP_Y;
}
/* @(/2/1) .................................................................*/
/* @(/2/1/4) ...............................................................*/
/* @(/2/1/4/0) */
static QState Ship_initial(Ship * const me, QEvt const * const e) {
    (void)e; /* avoid the compiler warning 'usused parameter' */
    QActive_subscribe((QActive *)me, TIME_TICK_SIG);
    QActive_subscribe((QActive *)me, PLAYER_TRIGGER_SIG);
    /* object dictionaries... */
    QS_OBJ_DICTIONARY(&l_ship);
    /* function dictionaries for Ship HSM... */
    QS_FUN_DICTIONARY(&Ship_initial);
    QS_FUN_DICTIONARY(&Ship_active);
    QS_FUN_DICTIONARY(&Ship_parked);
    QS_FUN_DICTIONARY(&Ship_flying);
    QS_FUN_DICTIONARY(&Ship_exploding);
    /* local signals... */
    QS_SIG_DICTIONARY(PLAYER_SHIP_MOVE_SIG, &l_ship);
    QS_SIG_DICTIONARY(TAKE_OFF_SIG,         &l_ship);
    QS_SIG_DICTIONARY(HIT_WALL_SIG,         &l_ship);
    QS_SIG_DICTIONARY(HIT_MINE_SIG,         &l_ship);
    QS_SIG_DICTIONARY(DESTROYED_MINE_SIG,   &l_ship);
    return Q_TRAN(&Ship_active);
}
/* @(/2/1/4/1) .............................................................*/
static QState Ship_active(Ship * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        /* @(/2/1/4/1/0) */
        case Q_INIT_SIG: {
            status = Q_TRAN(&Ship_parked);
            break;
        }
        /* @(/2/1/4/1/1) */
        case PLAYER_SHIP_MOVE_SIG: {
            me->x = Q_EVT_CAST(ObjectPosEvt)->x;
            me->y = Q_EVT_CAST(ObjectPosEvt)->y;
            status = Q_HANDLED();
            break;
        }
        default: {
            status = Q_SUPER(&QHsm_top);
            break;
        }
    }
    return status;
}
/* @(/2/1/4/1/2) ...........................................................*/
static QState Ship_parked(Ship * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        /* @(/2/1/4/1/2/0) */
        case TAKE_OFF_SIG: {
            status = Q_TRAN(&Ship_flying);
            break;
        }
        default: {
            status = Q_SUPER(&Ship_active);
            break;
        }
    }
    return status;
}
/* @(/2/1/4/1/3) ...........................................................*/
static QState Ship_flying(Ship * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        /* @(/2/1/4/1/3) */
        case Q_ENTRY_SIG: {
            me->score = 0U; /* reset the score */
            QACTIVE_POST(AO_Tunnel,
                (QEvt *)Q_NEW(ScoreEvt, SCORE_SIG, me->score),
                me);
            status = Q_HANDLED();
            break;
        }
        /* @(/2/1/4/1/3/0) */
        case TIME_TICK_SIG: {
            /* tell the Tunnel to draw the Ship and test for hits */
            QACTIVE_POST(AO_Tunnel,
                (QEvt *)Q_NEW(ObjectImageEvt, SHIP_IMG_SIG,
                              me->x, me->y, SHIP_BMP),
                me);

            ++me->score; /* increment the score for surviving another tick */

            if ((me->score % 10U) == 0U) { /* is the score "round"? */
                QACTIVE_POST(AO_Tunnel,
                    (QEvt *)Q_NEW(ScoreEvt, SCORE_SIG, me->score),
                    me);
            }
            status = Q_HANDLED();
            break;
        }
        /* @(/2/1/4/1/3/1) */
        case PLAYER_TRIGGER_SIG: {
            QACTIVE_POST(AO_Missile,
                (QEvt *)Q_NEW(ObjectPosEvt, MISSILE_FIRE_SIG,
                              me->x, me->y + SHIP_HEIGHT - 1U),
                me);
            status = Q_HANDLED();
            break;
        }
        /* @(/2/1/4/1/3/2) */
        case DESTROYED_MINE_SIG: {
            me->score += Q_EVT_CAST(ScoreEvt)->score;
            /* the score will be sent to the Tunnel by the next TIME_TICK */
            status = Q_HANDLED();
            break;
        }
        /* @(/2/1/4/1/3/3) */
        case HIT_WALL_SIG: {
            status = Q_TRAN(&Ship_exploding);
            break;
        }
        /* @(/2/1/4/1/3/4) */
        case HIT_MINE_SIG: {
            status = Q_TRAN(&Ship_exploding);
            break;
        }
        default: {
            status = Q_SUPER(&Ship_active);
            break;
        }
    }
    return status;
}
/* @(/2/1/4/1/4) ...........................................................*/
static QState Ship_exploding(Ship * const me, QEvt const * const e) {
    QState status;
    switch (e->sig) {
        /* @(/2/1/4/1/4) */
        case Q_ENTRY_SIG: {
            me->exp_ctr = 0U;
            status = Q_HANDLED();
            break;
        }
        /* @(/2/1/4/1/4/0) */
        case TIME_TICK_SIG: {
            /* @(/2/1/4/1/4/0/0) */
            if (me->exp_ctr < 15U) {
                ++me->exp_ctr;
                /* tell the Tunnel to draw the current stage of Explosion */
                QACTIVE_POST(AO_Tunnel,
                            (QEvt *)Q_NEW(ObjectImageEvt, EXPLOSION_SIG,
                                          me->x,
                                          (int8_t)((int)me->y - 4 + SHIP_HEIGHT),
                                          EXPLOSION0_BMP + (me->exp_ctr >> 2)),
                             me);
                status = Q_HANDLED();
            }
            /* @(/2/1/4/1/4/0/1) */
            else {
                QACTIVE_POST(AO_Tunnel,
                    (QEvt *)Q_NEW(ScoreEvt, GAME_OVER_SIG, me->score),
                    me);
                status = Q_TRAN(&Ship_parked);
            }
            break;
        }
        default: {
            status = Q_SUPER(&Ship_active);
            break;
        }
    }
    return status;
}

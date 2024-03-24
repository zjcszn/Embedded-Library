//============================================================================
// Product: "Blinky" example, Win32 API
// Last Updated for Version: 7.2.0
// Date of the Last Update:  2022-12-22
//
//                   Q u a n t u m     L e a P s
//                   ---------------------------
//                   innovating embedded systems
//
// Copyright (C) 2005 Quantum Leaps, LLC. All rights reserved.
//
// This program is open source software: you can redistribute it and/or
// modify it under the terms of the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Alternatively, this program may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GNU General Public License and are specifically designed for
// licensees interested in retaining the proprietary status of their code.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <www.gnu.org/licenses/>.
//
// Contact information:
// <www.state-machine.com/licensing>
// <info@state-machine.com>
//============================================================================
#include "qpc.h"
#include "bsp.h"
#include "gizmo.h"

//Q_DEFINE_THIS_FILE

static uint8_t status;

//............................................................................
void BSP_init() {
    QS_SIG_DICTIONARY(START_SIG, (void *)0);

    QS_OBJ_DICTIONARY(AO_Gizmo);
    QS_OBJ_DICTIONARY(&status);
}
//............................................................................
void BSP_msg(char const *msg) {
    QS_BEGIN_ID(QS_USER1, 0U)
       QS_STR(msg);
    QS_END()
}
//............................................................................
uint8_t BSP_getStatus(void) {
    return status;
}

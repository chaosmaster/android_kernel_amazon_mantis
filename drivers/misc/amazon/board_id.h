/*
 * board_id.h
 *
 * Copyright 2018 Amazon Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */


#define BOARD_ID_BIT_0 0
#define BOARD_ID_BIT_1 1
#define BOARD_ID_BIT_2 2

#define M_PVT   (1 << BOARD_ID_BIT_0)
#define M_DVT   (0)
#define M_EVT   (1 << BOARD_ID_BIT_2)
#define M_HVT   (1 << BOARD_ID_BIT_1 | 1 << BOARD_ID_BIT_0)
#define M_PROTO (1 << BOARD_ID_BIT_0 | 1 << BOARD_ID_BIT_1 | 1 << BOARD_ID_BIT_2)
#define M_DOE   (1 << BOARD_ID_BIT_2 | 1 << BOARD_ID_BIT_0)


/*
  Copyright (C) 2023 James Haley
  
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
*/

#pragma once

#define ELIB_WIN32_CONSOLE_PROGRAM

#define ELIB_MAIN_FN_DECL() \
    void ywctpatcher_main()

#define ELIB_MAIN_FN_CALL() \
    ywctpatcher_main()

#define ELIB_ORG_NAME "Team Eternity"
#define ELIB_APP_NAME "ywctpatcher"
#define ELIB_CFG_NAME "ywctpatcher.cfg"

// EOF

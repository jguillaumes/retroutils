//
//  logger.c
//  BlinkenServer
//
//  Created by Jordi Guillaumes Pons on 01/01/13.
//  Copyright (c) 2013 Jordi Guillaumes Pons. All rights reserved.
//

#include <stdio.h>
#include <syslog.h>

void initLogger() {
    openlog("BlinkenServer", LOG_CONS | LOG_PID, LOG_LOCAL0);
    syslog(LOG_WARNING, "%s", "BlinkenServer started");
}

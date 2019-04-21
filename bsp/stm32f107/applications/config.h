/*
 * File      : board.h
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006 - 2013, RT-Thread Development Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://www.rt-thread.org/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2009-09-22     Bernard      add board.h to this bsp
 */

// <<< Use Configuration Wizard in Context Menu >>>
#ifndef __CONFIG_H__
#define __CONFIG_H__

#define ET_SOFTWARE_VERSION     "1.0.8"
#define LOAD_BALANCE
//#define ET_TEST_MODE            1
#define WATCHDOG


//#define ET_SERVICE_URL        "test.ethome.com"
//#define ET_SERVICE_URL        "192.168.17.177"
//#define ET_SERVICE_URL          "yun.ethome.com"
#ifdef ET_TEST_MODE
#define SERVER_ADDR_URL         "test.ethome.com"
#define UDP_SERVICE_URL         "test.ethome.com"
#define UDP_SERVER_ADDR_URL     "test.ethome.com"
#define UDP_SERVER_PORT         "12900"
#define SIN_SERVER_PORT          12900
#else
#define SERVER_ADDR_URL         "yun.ethome.com"
#define UDP_SERVICE_URL         "udp.ethome.com"
#define UDP_SERVER_ADDR_URL     "udp.ethome.com"
#define UDP_SERVER_PORT         "12900"                       // for wifi module
#define SIN_SERVER_PORT         12900                         // for eth
#endif

#endif

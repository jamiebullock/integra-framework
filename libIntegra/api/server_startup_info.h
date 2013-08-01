/* libIntegra multimedia module interface
 *
 * Copyright (C) 2007 Birmingham City University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
 * USA.
 */

#ifndef INTEGRA_SERVER_STARTUP_INFO_H
#define INTEGRA_SERVER_STARTUP_INFO_H

#include "common_typedefs.h"

namespace ntg_api
{
	class LIBINTEGRA_API CServerStartupInfo
	{
		public:
			CServerStartupInfo()
			{
				bridge_path = "";
				system_module_directory = "";
				third_party_module_directory = "";
		
				xmlrpc_server_port = 0;
		
				osc_client_url = "";
				unsigned short osc_client_port = 0;
			}

			string bridge_path;
			string system_module_directory;
			string third_party_module_directory;
			
			unsigned short xmlrpc_server_port;
			
			string osc_client_url;
			unsigned short osc_client_port;
	};
}



#endif
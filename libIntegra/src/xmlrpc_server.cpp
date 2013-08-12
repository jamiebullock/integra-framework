/** libIntegra multimedia module interface
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


#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <pthread.h>

#ifdef _WINDOWS
#include <winsock2.h>
#endif

#include <xmlrpc-c/base.h>
#include <xmlrpc-c/abyss.h>
#include <xmlrpc-c/server.h>
#include <xmlrpc-c/server_abyss.h>


#include "platform_specifics.h"
#include "server.h"
#include "node.h"
#include "xmlrpc_common.h"
#include "xmlrpc_server.h"
#include "module_manager.h"
#include "interface_definition.h"
#include "api/guid_helper.h"
#include "string_helper.h"

#include "api/trace.h"
#include "api/error.h"
#include "api/value.h"
#include "api/path.h"
#include "api/common_typedefs.h"
#include "api/command_api.h"
#include "api/command_result.h"

#define HELPSTR_VERSION "Return the current version of libIntegra\n\\return {'response':'system.version', 'version':<string>\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}}\n"

#define HELPSTR_PRINTSTATE "Dump the entire node and attribute state to stdout for testing purposes\n\\return {'response':'system.printstate'\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}}\n"

#define HELPSTR_INTERFACELIST "Return a list of ids of interfaces available for instantiation on the server\n\\return {'response':'query.interfacelist', 'interfacelist':<array>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}}\n"

#define HELPSTR_INTERFACEINFO "Return interface info for a specified interface\n\\return {'response':'query.interfaceinfo', 'interfaceinfo':<struct>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}}\n"

#define HELPSTR_ENDPOINTS "Return endpoints for a specified interface\n\\return {'response':'query.endpoints', 'endpoints':<array>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}}\n"

#define HELPSTR_WIDGETS "Return widgets for a specified interface\n\\return {'response':'query.widgets', 'widgets':<array>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}}\n"

#define HELPSTR_NODELIST "Return a list of nodes under the current node using depth-first recursion\n\\param <array path>\n\\return {'response':'query.nodelist', 'nodelist':[{classname:<string>, path:<array>}, {classname:<string>, path:<array>}, ... ]}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_GET "Get the value of an attribute\n\\param <array attribute path>\n\\return {'response':'query.get', 'path':<array>, 'value':<value>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_SET "Set the value of an attribute\n\\param <array attribute path>\n\\param <value value>\n\\return {'response':'command.set', 'path':<array>, 'value':<value>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_NEW "Create a new node on the server\n\\param <string class name>\n\\param <string node name>\n\\param <array parent path>\n\\return {'response':'command.new', 'classname':<string>, 'instancename':<string>, 'parentpath':<array>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_DELETE "Delete a node from the server\n\\param <array path>\\return {'response':'command.delete', 'path':<array>\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_RENAME "Rename a node on the server\n\\param <array path>\n\\param <string node name>\n\\return {'response':'command.rename', 'instancepath':<array path>, 'name':<string name>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_SAVE "Save all nodes including and below a given node on the server to a path on the filesystem\n\\param <array path>\n\\param <string file path>\n\\return {'response':'command.save', 'nodepath':<array path>, 'filepath':<string path>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_LOAD "Load all nodes including and below a given node from the filesystem beneath the path given to the given node on the server\n\\param <string file path>\n\\param <array path>\n\\return {'response':'command.load', 'filepath':<string path>', 'parentpath':<array path>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_MOVE "Move a node on the server\n\\param <array node path>\n\\param <array new node parent path>\n\\return {'response':'command.move', 'instancepath':<array path>, 'parentpath':<array path>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_UNLOAD_ORPHANED_EMBEDDED "Remove orphaned embedded modules\n\\return {'response':'module.unloadorphanedembedded'}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_INSTALL_INTEGRA_MODULE_FILE "Install 3rd party module from module file\n\\param <string module file path>\n\\return {'response':'module.installintegramodulefile', moduleid:<string>, waspreviouslyembedded:<boolean>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_INSTALL_EMBEDDED_MODULE "Install embedded module\n\\param <string module guid>\n\\return {'response':'module.installembeddedmodule'}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_UNINSTALL_MODULE "Uninstall 3rd party module\n\\param <string module guid>\n\\return {'response':'module.uninstallmodule, remainsasembedded:<boolean>'}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"

#define HELPSTR_LOAD_MODULE_IN_DEVELOPMENT "Temporarily install module-in-development - only for this lifecycle of libIntegra\n\\param <string module file path>\n\\return {'response':'module.loadmoduleindevelopment', moduleid:<string>, previousmoduleid:<string, optional>, previousremainsasembedded<boolean, optional>}\n\\error {'response':'error', 'errorcode':<int>, 'errortext':<string>}\n"


#define RESPONSE_LABEL      "response"
#define NTG_NULL_STRING     "None"

/*define maximum size for incoming xmlrpc (default is 500Kb and can be exceeded by large requests)*/
#define NTG_XMLRPC_SIZE_LIMIT 1024 * 1000 * 1000

using namespace integra_api;
using namespace integra_internal;


static TServer abyssServer;
typedef xmlrpc_value *(* ntg_server_callback_va)( CServer *, int, va_list);



/* run server command -- blocking version
 * call this if you want a return value -- e.g. to XMLRPC interface */
xmlrpc_value *ntg_server_do_va( ntg_server_callback_va callback, CServer *server, const int argc, ...)
{
    xmlrpc_value *rv;
    va_list argv;

    server->lock();

    va_start(argv, argc);
    rv = callback( server, argc, argv);
    va_end(argv);

    server->unlock();

    return rv;
}

static xmlrpc_value *ntg_xmlrpc_error(xmlrpc_env * env,
        CError error)
{

    xmlrpc_value *struct_, *xmlrpc_temp;

    struct_ = xmlrpc_struct_new(env);

    xmlrpc_temp = xmlrpc_string_new(env, "error");
    if(xmlrpc_temp) {
        xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
        xmlrpc_DECREF(xmlrpc_temp);
    } else {
        INTEGRA_TRACE_ERROR << "getting xmlrpc RESPONSE_LABEL";
    }

    xmlrpc_temp = xmlrpc_int_new(env, error );
    if(xmlrpc_temp) {
        xmlrpc_struct_set_value(env, struct_, "errorcode", xmlrpc_temp);
        xmlrpc_DECREF(xmlrpc_temp);
    } else {
        INTEGRA_TRACE_ERROR << "getting xmlrpc errorcode";
    }

	xmlrpc_temp = xmlrpc_string_new(env, error.get_text().c_str() );
    if(xmlrpc_temp) {
        xmlrpc_struct_set_value(env, struct_, "errortext", xmlrpc_temp);
        xmlrpc_DECREF(xmlrpc_temp);
    } else {
        INTEGRA_TRACE_ERROR << "getting xmlrpc errortext";
    }
    return struct_;

}

static CPath ntg_xmlrpc_get_path( xmlrpc_env *env, xmlrpc_value *xmlrpc_path )
{
    int n;
    char *elem;
    int n_elems = 0;
    xmlrpc_value *xmlrpc_elem;

	CPath path;
    n_elems = xmlrpc_array_size(env, xmlrpc_path);

    for (n = 0; n < n_elems; n++) {
        xmlrpc_array_read_item(env, xmlrpc_path, n, &xmlrpc_elem);
        xmlrpc_read_string(env, xmlrpc_elem, (const char **)&elem);
        xmlrpc_DECREF(xmlrpc_elem);
		path.append_element( elem );
        assert(elem != NULL);
        free(elem);
    }

    return path;
}


static xmlrpc_value *ntg_xmlrpc_interfacelist_callback( CServer *server, const int argc, va_list argv)
{
    xmlrpc_value *array_ = NULL,
                 *array_item = NULL, *xmlrpc_name = NULL, *struct_ = NULL;
    xmlrpc_env *env;

    env = va_arg(argv, xmlrpc_env *);

	const guid_set &module_ids = server->get_module_manager().get_all_module_ids();

    array_ = xmlrpc_array_new(env);
	for( guid_set::const_iterator i = module_ids.begin(); i != module_ids.end(); i++ ) 
	{
		string module_id_string = CGuidHelper::guid_to_string( *i );
        array_item = xmlrpc_string_new( env, module_id_string.c_str() );
        xmlrpc_array_append_item(env, array_, array_item);
        xmlrpc_DECREF(array_item);
    }

    if (env->fault_occurred)
        return NULL;

    struct_ = xmlrpc_struct_new(env);
    xmlrpc_name = xmlrpc_string_new(env, "query.interfacelist");

    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_name);
    xmlrpc_struct_set_value(env, struct_, "interfacelist", array_);

    xmlrpc_DECREF(xmlrpc_name);
    xmlrpc_DECREF(array_);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_interfaceinfo_callback( CServer *server, const int argc, va_list argv)
{
	xmlrpc_value *info_struct = NULL, *xmlrpc_temp = NULL, *xmlrpc_array = NULL, *struct_ = NULL;
    xmlrpc_env *env;
    char *module_id_string;
	GUID guid;

    env = va_arg(argv, xmlrpc_env *);
    module_id_string = va_arg(argv, char *);

    if (env->fault_occurred)
	{
        return NULL;
	}

	if( CGuidHelper::string_to_guid( module_id_string, guid ) != CError::SUCCESS ) 
	{
	    free(module_id_string);
		return ntg_xmlrpc_error(env, CError::INPUT_ERROR);
	}

	const CInterfaceDefinition *interface_definition = server->get_module_manager().get_interface_by_module_id( guid );
    if( !interface_definition )	
	{
	    free( module_id_string );
		return ntg_xmlrpc_error(env, CError::INPUT_ERROR);
	}

	const CInterfaceInfo &info = interface_definition->get_interface_info();

    info_struct = xmlrpc_struct_new(env);
    struct_ = xmlrpc_struct_new(env);

    xmlrpc_temp = xmlrpc_string_new(env, info.get_name().c_str() );
    xmlrpc_struct_set_value(env, info_struct, "name", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

	xmlrpc_temp = xmlrpc_string_new(env, info.get_label().c_str() );
    xmlrpc_struct_set_value(env, info_struct, "label", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

	xmlrpc_temp = xmlrpc_string_new(env, info.get_description().c_str() );
    xmlrpc_struct_set_value(env, info_struct, "description", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

	xmlrpc_array = xmlrpc_array_new(env);
	for( string_set::const_iterator i = info.get_tags().begin(); i != info.get_tags().end(); i++ )
	{
		xmlrpc_temp = xmlrpc_string_new( env, i->c_str() );
		xmlrpc_array_append_item( env, xmlrpc_array, xmlrpc_temp );
		xmlrpc_DECREF(xmlrpc_temp);
	}
	xmlrpc_struct_set_value(env, info_struct, "tags", xmlrpc_array );
	xmlrpc_DECREF(xmlrpc_array);

	xmlrpc_temp = xmlrpc_int_new(env, info.get_implemented_in_libintegra() ? 1 : 0 );
    xmlrpc_struct_set_value(env, info_struct, "implementedinlibintegra", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

	xmlrpc_temp = xmlrpc_string_new(env, info.get_author().c_str() );
	xmlrpc_struct_set_value(env, info_struct, "author", xmlrpc_temp);
	xmlrpc_DECREF(xmlrpc_temp);

	string date_string = CStringHelper::date_to_string( info.get_created_date() );
	xmlrpc_temp = xmlrpc_string_new(env, date_string.c_str() );
	xmlrpc_struct_set_value(env, info_struct, "createddate", xmlrpc_temp);
	xmlrpc_DECREF(xmlrpc_temp);

	date_string = CStringHelper::date_to_string( info.get_modified_date() );
	xmlrpc_temp = xmlrpc_string_new(env, date_string.c_str() );
	xmlrpc_struct_set_value(env, info_struct, "modifieddate", xmlrpc_temp);
	xmlrpc_DECREF(xmlrpc_temp);

	/* now package the built struct... */

    xmlrpc_temp = xmlrpc_string_new(env, "query.interfaceinfo");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_temp = xmlrpc_string_new(env, module_id_string);
    xmlrpc_struct_set_value(env, struct_, "moduleid", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

	string origin_id_string = CGuidHelper::guid_to_string( interface_definition->get_origin_guid() );
    xmlrpc_temp = xmlrpc_string_new( env, origin_id_string.c_str() );
    xmlrpc_struct_set_value(env, struct_, "originid", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

	switch( interface_definition->get_module_source() )
	{
		case CInterfaceDefinition::MODULE_SHIPPED_WITH_INTEGRA:
			xmlrpc_temp = xmlrpc_string_new(env, "shippedwithintegra" );
			break;

		case CInterfaceDefinition::MODULE_3RD_PARTY:
			xmlrpc_temp = xmlrpc_string_new(env, "thirdparty" );
			break;

		case CInterfaceDefinition::MODULE_EMBEDDED:
			xmlrpc_temp = xmlrpc_string_new(env, "embedded" );
			break;

		case CInterfaceDefinition::MODULE_IN_DEVELOPMENT:
			xmlrpc_temp = xmlrpc_string_new(env, "indevelopment" );
			break;

		default:
			assert( false );
			break;
	}

	xmlrpc_struct_set_value(env, struct_, "modulesource", xmlrpc_temp);
	xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_struct_set_value(env, struct_, "interfaceinfo", info_struct);
    xmlrpc_DECREF(info_struct);

	const CImplementationInfo *implementation_info = interface_definition->get_implementation_info();
	if( implementation_info )
	{
		xmlrpc_temp = xmlrpc_int_new( env, ( int ) implementation_info->get_checksum() );
		xmlrpc_struct_set_value( env, struct_, "implementationchecksum", xmlrpc_temp );
		xmlrpc_DECREF(xmlrpc_temp);
	}

    /* free out-of-place memory */
    free(module_id_string);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_endpoints_callback( CServer *server, const int argc, va_list argv )
{
	xmlrpc_value *xmlrpc_temp = NULL, *endpoints_array = NULL, *xmlrpc_endpoint = NULL, *struct_ = NULL;
	xmlrpc_value *xmlrpc_control_info = NULL, *xmlrpc_state_info = NULL, *xmlrpc_stream_info = NULL;
	xmlrpc_value *xmlrpc_constraint, *xmlrpc_range, *xmlrpc_scale, *xmlrpc_scale_exponent_root, *xmlrpc_allowed_states, *xmlrpc_state_labels = NULL, *xmlrpc_state_label = NULL;
    xmlrpc_env *env;
    char *module_id_string;
	GUID guid;

    env = va_arg(argv, xmlrpc_env *);
    module_id_string = va_arg(argv, char *);

    if (env->fault_occurred)
	{
        return NULL;
	}

	if( CGuidHelper::string_to_guid( module_id_string, guid ) != CError::SUCCESS ) 
	{
	    free(module_id_string);
		return ntg_xmlrpc_error(env, CError::INPUT_ERROR);
	}

	const CInterfaceDefinition *interface_definition = server->get_module_manager().get_interface_by_module_id( guid );
    if( !interface_definition )	
	{
	    free(module_id_string);
		return ntg_xmlrpc_error(env, CError::INPUT_ERROR);
	}

    endpoints_array = xmlrpc_array_new(env);
    struct_ = xmlrpc_struct_new(env);

	const endpoint_definition_list &endpoint_definitions = interface_definition->get_endpoint_definitions();

	for( endpoint_definition_list::const_iterator i = endpoint_definitions.begin(); i != endpoint_definitions.end(); i++ )
	{
		const CEndpointDefinition &endpoint_definition = **i;

		xmlrpc_endpoint = xmlrpc_struct_new( env );

		xmlrpc_temp = xmlrpc_string_new( env, endpoint_definition.get_name().c_str() );
		xmlrpc_struct_set_value(env, xmlrpc_endpoint, "name", xmlrpc_temp);
		xmlrpc_DECREF(xmlrpc_temp);

		xmlrpc_temp = xmlrpc_string_new(env, endpoint_definition.get_label().c_str() );
		xmlrpc_struct_set_value(env, xmlrpc_endpoint, "label", xmlrpc_temp);
		xmlrpc_DECREF(xmlrpc_temp);

		xmlrpc_temp = xmlrpc_string_new(env, endpoint_definition.get_description().c_str() );
		xmlrpc_struct_set_value(env, xmlrpc_endpoint, "description", xmlrpc_temp);
		xmlrpc_DECREF(xmlrpc_temp);

		switch( endpoint_definition.get_type() )
		{
			case CEndpointDefinition::CONTROL:
				xmlrpc_temp = xmlrpc_string_new(env, "control" );
				break;

			case CEndpointDefinition::STREAM:
				xmlrpc_temp = xmlrpc_string_new(env, "stream" );
				break;

			default:
				assert( false );
				continue;
		}

		xmlrpc_struct_set_value(env, xmlrpc_endpoint, "type", xmlrpc_temp);
		xmlrpc_DECREF(xmlrpc_temp);

		const CControlInfo *control_info = endpoint_definition.get_control_info();
		if( control_info )
		{
			xmlrpc_control_info = xmlrpc_struct_new( env );

			switch( control_info->get_type() )
			{
				case CControlInfo::STATEFUL:
					xmlrpc_temp = xmlrpc_string_new(env, "state" );
					break;

				case CControlInfo::BANG:
					xmlrpc_temp = xmlrpc_string_new(env, "bang" );
					break;

				default:
					assert( false );
					continue;
			}
			xmlrpc_struct_set_value(env, xmlrpc_control_info, "type", xmlrpc_temp);
			xmlrpc_DECREF(xmlrpc_temp);

			const CStateInfo *state_info = control_info->get_state_info();
			if( state_info )
			{
				xmlrpc_state_info = xmlrpc_struct_new( env );

				switch( state_info->get_type() )
				{
					case CValue::INTEGER:
						xmlrpc_temp = xmlrpc_string_new(env, "integer" );
						break;

					case CValue::FLOAT:
						xmlrpc_temp = xmlrpc_string_new(env, "float" );
						break;

					case CValue::STRING:
						xmlrpc_temp = xmlrpc_string_new(env, "string" );
						break;

					default:
						assert( false );
						continue;
				}

				xmlrpc_struct_set_value(env, xmlrpc_state_info, "type", xmlrpc_temp);
				xmlrpc_DECREF(xmlrpc_temp);
				
				xmlrpc_constraint = xmlrpc_struct_new( env );

				const CValueRange *value_range = state_info->get_constraint().get_value_range();
				if( value_range )
				{
					xmlrpc_range = xmlrpc_struct_new( env );
					
					xmlrpc_temp = ntg_xmlrpc_value_new( value_range->get_minimum(), env );
					xmlrpc_struct_set_value( env, xmlrpc_range, "minimum", xmlrpc_temp);
					xmlrpc_DECREF(xmlrpc_temp);

					xmlrpc_temp = ntg_xmlrpc_value_new( value_range->get_maximum(), env );
					xmlrpc_struct_set_value( env, xmlrpc_range, "maximum", xmlrpc_temp);
					xmlrpc_DECREF(xmlrpc_temp);

					xmlrpc_struct_set_value(env, xmlrpc_constraint, "range", xmlrpc_range);
					xmlrpc_DECREF(xmlrpc_range);
				}

				const value_set *allowed_states = state_info->get_constraint().get_allowed_states();
				if( allowed_states )
				{
					xmlrpc_allowed_states = xmlrpc_array_new( env );

					for( value_set::const_iterator i = allowed_states->begin(); i != allowed_states->end(); i++ )
					{
						xmlrpc_temp = ntg_xmlrpc_value_new( **i, env );
						xmlrpc_array_append_item( env, xmlrpc_allowed_states, xmlrpc_temp );
						xmlrpc_DECREF(xmlrpc_temp);
					}

					xmlrpc_struct_set_value(env, xmlrpc_constraint, "allowedstates", xmlrpc_allowed_states);
					xmlrpc_DECREF( xmlrpc_allowed_states );
				}

				xmlrpc_struct_set_value(env, xmlrpc_state_info, "constraint", xmlrpc_constraint );
				xmlrpc_DECREF(xmlrpc_constraint);

				const CValue &default_value = state_info->get_default_value();
				if( default_value.get_type() != state_info->get_type() )
				{
					INTEGRA_TRACE_ERROR << "default value missing or of incorrect type: " << endpoint_definition.get_name();
				}
				else
				{
					xmlrpc_temp = ntg_xmlrpc_value_new( default_value, env );
					xmlrpc_struct_set_value( env, xmlrpc_state_info, "defaultvalue", xmlrpc_temp );
					xmlrpc_DECREF(xmlrpc_temp);
				}

				const CValueScale *value_scale = state_info->get_value_scale();
				if( value_scale )
				{
					xmlrpc_scale = xmlrpc_struct_new( env );

					switch( value_scale->get_scale_type() )
					{
						case CValueScale::LINEAR:
							xmlrpc_temp = xmlrpc_string_new(env, "linear" );
							xmlrpc_scale_exponent_root = NULL;
							break;

						case CValueScale::EXPONENTIAL:
							xmlrpc_temp = xmlrpc_string_new(env, "exponential" );
							xmlrpc_scale_exponent_root = xmlrpc_int_new( env, value_scale->get_exponent_root() );
							break;

						case CValueScale::DECIBEL:
							xmlrpc_temp = xmlrpc_string_new(env, "decibel" );
							xmlrpc_scale_exponent_root = NULL;
							break;

						default:
							assert( false );
							continue;
					}

					xmlrpc_struct_set_value( env, xmlrpc_scale, "type", xmlrpc_temp );
					xmlrpc_DECREF( xmlrpc_temp );

					if( xmlrpc_scale_exponent_root )
					{
						xmlrpc_struct_set_value( env, xmlrpc_scale, "base", xmlrpc_scale_exponent_root );
						xmlrpc_DECREF( xmlrpc_scale_exponent_root );
					}

					xmlrpc_struct_set_value( env, xmlrpc_state_info, "scale", xmlrpc_scale );
				}

				xmlrpc_state_labels = xmlrpc_array_new( env );
				const value_map &state_labels = state_info->get_state_labels();
				for( value_map::const_iterator i = state_labels.begin(); i != state_labels.end(); i++ )
				{
					xmlrpc_state_label = xmlrpc_struct_new( env );

					xmlrpc_temp = ntg_xmlrpc_value_new( *i->second, env );
					xmlrpc_struct_set_value( env, xmlrpc_state_label, "value", xmlrpc_temp );
					xmlrpc_DECREF(xmlrpc_temp);

					xmlrpc_temp = xmlrpc_string_new( env, i->first.c_str() );
					xmlrpc_struct_set_value( env, xmlrpc_state_label, "text", xmlrpc_temp );
					xmlrpc_DECREF(xmlrpc_temp);

					xmlrpc_array_append_item( env, xmlrpc_state_labels, xmlrpc_state_label );
					xmlrpc_DECREF(xmlrpc_state_label);
				}

				xmlrpc_struct_set_value( env, xmlrpc_state_info, "statelabels", xmlrpc_state_labels );
				xmlrpc_DECREF(xmlrpc_state_labels);

				xmlrpc_temp = xmlrpc_int_new( env, state_info->get_is_input_file() ? 1 : 0 );
				xmlrpc_struct_set_value( env, xmlrpc_state_info, "isinputfile", xmlrpc_temp );
				xmlrpc_DECREF(xmlrpc_temp);

				xmlrpc_temp = xmlrpc_int_new( env, state_info->get_is_saved_to_file() ? 1 : 0 );
				xmlrpc_struct_set_value( env, xmlrpc_state_info, "issavedtofile", xmlrpc_temp );
				xmlrpc_DECREF(xmlrpc_temp);

				/* store built state info struct */
				xmlrpc_struct_set_value(env, xmlrpc_control_info, "stateinfo", xmlrpc_state_info);
				xmlrpc_DECREF(xmlrpc_state_info);
			}

			xmlrpc_temp = xmlrpc_int_new( env, control_info->get_can_be_source() ? 1 : 0 );
			xmlrpc_struct_set_value(env, xmlrpc_control_info, "canbesource", xmlrpc_temp);
			xmlrpc_DECREF(xmlrpc_temp);

			xmlrpc_temp = xmlrpc_int_new( env, control_info->get_can_be_target() ? 1 : 0 );
			xmlrpc_struct_set_value(env, xmlrpc_control_info, "canbetarget", xmlrpc_temp);
			xmlrpc_DECREF(xmlrpc_temp);

			xmlrpc_temp = xmlrpc_int_new( env, control_info->get_is_sent_to_host() ? 1 : 0 );
			xmlrpc_struct_set_value(env, xmlrpc_control_info, "issenttohost", xmlrpc_temp);
			xmlrpc_DECREF(xmlrpc_temp);

			/* store built control info struct */
			xmlrpc_struct_set_value(env, xmlrpc_endpoint, "controlinfo", xmlrpc_control_info);
			xmlrpc_DECREF(xmlrpc_control_info);
		}

		const CStreamInfo *stream_info = endpoint_definition.get_stream_info();
		if( stream_info )
		{
			xmlrpc_stream_info = xmlrpc_struct_new( env );

			switch( stream_info->get_type() )
			{
				case CStreamInfo::AUDIO:
					xmlrpc_temp = xmlrpc_string_new(env, "audio" );
					break;

				default:
					assert( false );
					continue;
			}

			xmlrpc_struct_set_value(env, xmlrpc_stream_info, "type", xmlrpc_temp);
			xmlrpc_DECREF(xmlrpc_temp);

			switch( stream_info->get_direction() )
			{
				case CStreamInfo::INPUT:
					xmlrpc_temp = xmlrpc_string_new(env, "input" );
					break;

				case CStreamInfo::OUTPUT:
					xmlrpc_temp = xmlrpc_string_new(env, "output" );
					break;

				default:
					assert( false );
					continue;
			}

			xmlrpc_struct_set_value(env, xmlrpc_stream_info, "direction", xmlrpc_temp);
			xmlrpc_DECREF(xmlrpc_temp);

			/* store built stream info struct */
			xmlrpc_struct_set_value(env, xmlrpc_endpoint, "streaminfo", xmlrpc_stream_info);
			xmlrpc_DECREF(xmlrpc_stream_info);
		}

		xmlrpc_array_append_item( env, endpoints_array, xmlrpc_endpoint );
		xmlrpc_DECREF(xmlrpc_endpoint);
	}

	/* now package the built struct... */

    xmlrpc_temp = xmlrpc_string_new(env, "query.endpoints");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_temp = xmlrpc_string_new(env, module_id_string);
    xmlrpc_struct_set_value(env, struct_, "moduleid", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_struct_set_value(env, struct_, "endpoints", endpoints_array);
    xmlrpc_DECREF(endpoints_array);

    /* free out-of-place memory */
    free(module_id_string);

	return struct_;
}


static xmlrpc_value *ntg_xmlrpc_widgets_callback( CServer *server, const int argc, va_list argv )
{
	xmlrpc_value *xmlrpc_temp = NULL, *widgets_array = NULL, *xmlrpc_widget, *xmlrpc_position, *struct_ = NULL;
	xmlrpc_value *xmlrpc_mapping_list, *xmlrpc_mapping;
    xmlrpc_env *env;
    char *module_id_string;
	GUID guid;

    env = va_arg(argv, xmlrpc_env *);
    module_id_string = va_arg(argv, char *);

    if (env->fault_occurred)
	{
        return NULL;
	}

	if( CGuidHelper::string_to_guid( module_id_string, guid ) != CError::SUCCESS ) 
	{
	    free(module_id_string);
		return ntg_xmlrpc_error(env, CError::INPUT_ERROR);
	}

	const CInterfaceDefinition *interface_definition = server->get_module_manager().get_interface_by_module_id( guid );
    if( !interface_definition )	
	{
	    free(module_id_string);
		return ntg_xmlrpc_error(env, CError::INPUT_ERROR);
	}

    struct_ = xmlrpc_struct_new( env );

	widgets_array = xmlrpc_array_new( env );
	
	const widget_definition_list &widget_definitions = interface_definition->get_widget_definitions();
	for( widget_definition_list::const_iterator i = widget_definitions.begin(); i != widget_definitions.end(); i++ )
	{
		const CWidgetDefinition &widget_definition = **i;

		xmlrpc_widget = xmlrpc_struct_new( env );

		xmlrpc_temp = xmlrpc_string_new( env, widget_definition.get_type().c_str() );
		xmlrpc_struct_set_value(env, xmlrpc_widget, "type", xmlrpc_temp);
		xmlrpc_DECREF(xmlrpc_temp);

		xmlrpc_temp = xmlrpc_string_new( env, widget_definition.get_label().c_str() );
		xmlrpc_struct_set_value(env, xmlrpc_widget, "label", xmlrpc_temp);
		xmlrpc_DECREF(xmlrpc_temp);

		xmlrpc_position = xmlrpc_struct_new( env );

		const CWidgetPosition &widget_position = widget_definition.get_position();
		xmlrpc_temp = xmlrpc_double_new( env, widget_position.get_x() );
		xmlrpc_struct_set_value(env, xmlrpc_position, "x", xmlrpc_temp);
		xmlrpc_DECREF(xmlrpc_temp);

		xmlrpc_temp = xmlrpc_double_new( env, widget_position.get_y() );
		xmlrpc_struct_set_value(env, xmlrpc_position, "y", xmlrpc_temp);
		xmlrpc_DECREF(xmlrpc_temp);

		xmlrpc_temp = xmlrpc_double_new( env, widget_position.get_width() );
		xmlrpc_struct_set_value(env, xmlrpc_position, "width", xmlrpc_temp);
		xmlrpc_DECREF(xmlrpc_temp);

		xmlrpc_temp = xmlrpc_double_new( env, widget_position.get_height() );
		xmlrpc_struct_set_value(env, xmlrpc_position, "height", xmlrpc_temp);
		xmlrpc_DECREF(xmlrpc_temp);

		xmlrpc_struct_set_value(env, xmlrpc_widget, "position", xmlrpc_position);
		xmlrpc_DECREF(xmlrpc_position);

		xmlrpc_mapping_list = xmlrpc_array_new( env );
		const string_map &attribute_mappings = widget_definition.get_attribute_mappings();
		for( string_map::const_iterator i = attribute_mappings.begin(); i != attribute_mappings.end(); i++ )
		{
			xmlrpc_mapping = xmlrpc_struct_new( env );

			xmlrpc_temp = xmlrpc_string_new( env, i->first.c_str() );
			xmlrpc_struct_set_value(env, xmlrpc_mapping, "widgetattribute", xmlrpc_temp);
			xmlrpc_DECREF(xmlrpc_temp);

			xmlrpc_temp = xmlrpc_string_new( env, i->second.c_str() );
			xmlrpc_struct_set_value(env, xmlrpc_mapping, "endpoint", xmlrpc_temp);
			xmlrpc_DECREF(xmlrpc_temp);

			xmlrpc_array_append_item( env, xmlrpc_mapping_list, xmlrpc_mapping );
			xmlrpc_DECREF( xmlrpc_mapping );
		}

		xmlrpc_struct_set_value(env, xmlrpc_widget, "mapping", xmlrpc_mapping_list);
		xmlrpc_DECREF( xmlrpc_mapping_list );

		xmlrpc_array_append_item( env, widgets_array, xmlrpc_widget );
		xmlrpc_DECREF( xmlrpc_widget );
	}

	/* now package the built struct... */

    xmlrpc_temp = xmlrpc_string_new(env, "query.widgets");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_temp = xmlrpc_string_new(env, module_id_string);
    xmlrpc_struct_set_value(env, struct_, "moduleid", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_struct_set_value(env, struct_, "widgets", widgets_array);
    xmlrpc_DECREF(widgets_array);

    /* free out-of-place memory */
    free(module_id_string);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_nodelist_callback( CServer *server, const int argc, va_list argv )
{
	xmlrpc_env *env;
    xmlrpc_value *xmlrpc_nodes = NULL,
                 *xmlrpc_path = NULL,
                 *xmlrpc_elem = NULL,
                 *xmlrpc_guid = NULL,
                 *xmlrpc_temp = NULL, *struct_ = NULL, *node_struct = NULL;

    env = va_arg(argv, xmlrpc_env *);
    const CPath *path = va_arg(argv, CPath *);

    xmlrpc_nodes = xmlrpc_array_new(env);
    struct_ = xmlrpc_struct_new(env);

	path_list paths;
	const CNode *parent = server->find_node( *path );
	const node_map &nodes = parent ? parent->get_children() : server->get_nodes();
	for( node_map::const_iterator i = nodes.begin(); i != nodes.end(); i++ )
	{
		i->second->get_all_node_paths( paths );
	}

    /* each node is a ntg_path */
	for( path_list::const_iterator path_iterator = paths.begin(); path_iterator != paths.end(); path_iterator++ )
	{
        xmlrpc_path = xmlrpc_array_new(env);

		const CPath &path = *path_iterator;

		for( int i = 0; i < path.get_number_of_elements(); i++ )
		{
			xmlrpc_elem = xmlrpc_string_new( env, path[ i ].c_str() );

            xmlrpc_array_append_item(env, xmlrpc_path, xmlrpc_elem);
            xmlrpc_DECREF(xmlrpc_elem);
        }

        /* get the class id */
        const CNode *node = server->find_node( path );
        if( node == NULL ) 
		{
			INTEGRA_TRACE_ERROR << "path not found: " << path.get_string().c_str();
            return ntg_xmlrpc_error(env, CError::FAILED);
        }

		string module_id_string = CGuidHelper::guid_to_string( node->get_interface_definition().get_module_guid() );

        /* construct the node struct and append to nodes array */
        node_struct = xmlrpc_struct_new(env);
        xmlrpc_guid = xmlrpc_string_new(env, module_id_string.c_str() );
        xmlrpc_struct_set_value(env, node_struct, "guid", xmlrpc_guid);
        xmlrpc_DECREF(xmlrpc_guid);
        xmlrpc_struct_set_value(env, node_struct, "path", xmlrpc_path);
        xmlrpc_DECREF(xmlrpc_path);
        xmlrpc_array_append_item(env, xmlrpc_nodes, node_struct);
        xmlrpc_DECREF(node_struct);
    }

    xmlrpc_temp = xmlrpc_string_new(env, "query.nodelist");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_struct_set_value(env, struct_, "nodelist", xmlrpc_nodes);
    xmlrpc_DECREF(xmlrpc_nodes);

    return struct_;

}

static xmlrpc_value *ntg_xmlrpc_version_callback( CServer *server, const int argc, va_list argv )
{
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL, *xmlrpc_temp = NULL;

    env = va_arg(argv, xmlrpc_env *);

    struct_ = xmlrpc_struct_new(env);

	string version = server->get_libintegra_version();

    xmlrpc_temp = xmlrpc_string_new(env, "system.version");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_temp = xmlrpc_string_new(env, version.c_str() );
    xmlrpc_struct_set_value(env, struct_, "version", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_print_state_callback( CServer *server, const int argc, va_list argv )
{
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL, *xmlrpc_temp = NULL;

    env = va_arg(argv, xmlrpc_env *);

    struct_ = xmlrpc_struct_new(env);

	server->dump_state();

    xmlrpc_temp = xmlrpc_string_new(env, "system.printstate");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_delete_callback( CServer *server, const int argc, va_list argv )
{


    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL, *xmlrpc_temp = NULL, *xmlrpc_path = NULL;

    env = va_arg(argv, xmlrpc_env *);
    const CPath *path = va_arg(argv, CPath *);

    struct_ = xmlrpc_struct_new(env);

	CError error = server->process_command( CDeleteCommandApi::create( *path ), CCommandSource::XMLRPC_API );
    if(error != CError::SUCCESS) 
	{
        return ntg_xmlrpc_error(env, error);
    }

    xmlrpc_path = ntg_xmlrpc_value_from_path( *path, env );

    xmlrpc_temp = xmlrpc_string_new(env, "command.delete");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_struct_set_value(env, struct_, "path", xmlrpc_path);
    xmlrpc_DECREF(xmlrpc_path);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_rename_callback( CServer *server, const int argc, va_list argv )
{
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL, *xmlrpc_temp = NULL, *xmlrpc_path = NULL;

    env = va_arg(argv, xmlrpc_env *);

    struct_ = xmlrpc_struct_new(env);
    const CPath *path = va_arg(argv, CPath *);
    const char *name = va_arg(argv, char *);

	CError error = server->process_command( CRenameCommandApi::create( *path, name ), CCommandSource::XMLRPC_API );
    if( error != CError::SUCCESS ) 
	{
        return ntg_xmlrpc_error(env, error );
    }

    xmlrpc_path = ntg_xmlrpc_value_from_path( *path, env );

    xmlrpc_temp = xmlrpc_string_new(env, "command.rename");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_struct_set_value(env, struct_, "instancepath", xmlrpc_path);
    xmlrpc_DECREF(xmlrpc_path);

    xmlrpc_temp = xmlrpc_string_new(env, name);
    xmlrpc_struct_set_value(env, struct_, "name", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_save_callback( CServer *server, const int argc, va_list argv )
{
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL, *xmlrpc_temp = NULL, *xmlrpc_path = NULL;

    env = va_arg(argv, xmlrpc_env *);

    struct_ = xmlrpc_struct_new(env);
    const CPath *path = va_arg(argv, CPath  *);
    const char *file_path = va_arg(argv, char *);

	CError error = server->process_command( CSaveCommandApi::create( file_path, *path ), CCommandSource::XMLRPC_API );
    if( error != CError::SUCCESS ) 
	{
        return ntg_xmlrpc_error(env, error);
    }

    xmlrpc_temp = xmlrpc_string_new(env, "command.save");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_path = ntg_xmlrpc_value_from_path( *path, env);
    xmlrpc_struct_set_value(env, struct_, "instancepath", xmlrpc_path);
    xmlrpc_DECREF(xmlrpc_path);

    xmlrpc_temp = xmlrpc_string_new(env, file_path);
    xmlrpc_struct_set_value(env, struct_, "filepath", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_load_callback( CServer *server, const int argc, va_list argv )
{
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL, *xmlrpc_temp = NULL, *xmlrpc_path = NULL, *xmlrpc_array = NULL;

    env = va_arg(argv, xmlrpc_env *);

    struct_ = xmlrpc_struct_new(env);
    const char *file_path = va_arg(argv, char *);
    const CPath *path = va_arg(argv, CPath *);

	CLoadCommandResult result;
	CError error = server->process_command( CLoadCommandApi::create( file_path, *path ), CCommandSource::XMLRPC_API, &result );
    if( error != CError::SUCCESS ) 
	{
        return ntg_xmlrpc_error( env, error );
    }

	xmlrpc_temp = xmlrpc_string_new(env, "command.load");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_temp = xmlrpc_string_new(env, file_path);
    xmlrpc_struct_set_value(env, struct_, "filepath", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_path = ntg_xmlrpc_value_from_path( *path, env );
    xmlrpc_struct_set_value(env, struct_, "parentpath", xmlrpc_path);
    xmlrpc_DECREF(xmlrpc_path);

	xmlrpc_array = xmlrpc_array_new( env );

	const guid_set &new_embedded_module_ids = result.get_new_embedded_module_ids();
	for( guid_set::const_iterator i = new_embedded_module_ids.begin(); i != new_embedded_module_ids.end(); i++ )
	{
		string module_id_string = CGuidHelper::guid_to_string( *i );
		xmlrpc_temp = xmlrpc_string_new( env, module_id_string.c_str() );
		xmlrpc_array_append_item( env, xmlrpc_array, xmlrpc_temp);
		xmlrpc_DECREF( xmlrpc_temp );
	}

    xmlrpc_struct_set_value(env, struct_, "embeddedmodules", xmlrpc_array );

    xmlrpc_DECREF(xmlrpc_array);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_move_callback( CServer *server, const int argc, va_list argv )
{
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL,
                 *xmlrpc_node_path = NULL,
                 *xmlrpc_temp = NULL, *xmlrpc_parent_path = NULL;

    env = va_arg(argv, xmlrpc_env *);

    struct_ = xmlrpc_struct_new(env);
    const CPath *node_path = va_arg(argv, CPath *);
    const CPath *parent_path = va_arg(argv, CPath *);

	CError error = server->process_command( CMoveCommandApi::create( *node_path, *parent_path ), CCommandSource::XMLRPC_API );
    if( error != CError::SUCCESS ) 
	{
        return ntg_xmlrpc_error( env, error );
    }

    xmlrpc_node_path = ntg_xmlrpc_value_from_path( *node_path, env );
    xmlrpc_parent_path = ntg_xmlrpc_value_from_path( *parent_path, env );

    xmlrpc_temp = xmlrpc_string_new(env, "command.move");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_struct_set_value(env, struct_, "instancepath", xmlrpc_node_path);
    xmlrpc_DECREF(xmlrpc_node_path);

    xmlrpc_struct_set_value(env, struct_, "parentpath", xmlrpc_parent_path);
    xmlrpc_DECREF(xmlrpc_parent_path);

    return struct_;

}


static xmlrpc_value *ntg_xmlrpc_unload_orphaned_embedded_callback( CServer *server, const int argc, va_list argv )
{
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL, *xmlrpc_temp = NULL;

    env = va_arg(argv, xmlrpc_env *);

    struct_ = xmlrpc_struct_new(env);

	CError error = server->get_module_manager_writable().unload_orphaned_embedded_modules();

    if (error != CError::SUCCESS ) 
	{
        return ntg_xmlrpc_error( env, error );
    }

    xmlrpc_temp = xmlrpc_string_new(env, "module.unloadorphanedembedded" );
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_install_module_callback( CServer *server, const int argc, va_list argv )
{
    char *file_path;
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL, *xmlrpc_temp = NULL;

	assert( server );

    env = va_arg( argv, xmlrpc_env * );
    file_path = va_arg( argv, char * );

	CModuleInstallResult result;
	CError error = server->get_module_manager_writable().install_module( file_path, result );

	if( error != CError::SUCCESS )
	{
        free( file_path );
		return ntg_xmlrpc_error( env, error );
	}

    struct_ = xmlrpc_struct_new( env );

	string module_id_string = CGuidHelper::guid_to_string( result.module_id );

    xmlrpc_temp = xmlrpc_string_new( env, "module.installintegramodulefile" );
    xmlrpc_struct_set_value( env, struct_, RESPONSE_LABEL, xmlrpc_temp );
    xmlrpc_DECREF( xmlrpc_temp );

    xmlrpc_temp = xmlrpc_string_new(env, module_id_string.c_str() );
    xmlrpc_struct_set_value(env, struct_, "moduleid", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

	xmlrpc_temp = xmlrpc_bool_new(env, result.was_previously_embedded );
    xmlrpc_struct_set_value(env, struct_, "waspreviouslyembedded", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    /* free out-of-place memory */
    free( file_path );

    return struct_;
}



static xmlrpc_value *ntg_xmlrpc_load_module_in_development_callback( CServer *server, const int argc, va_list argv )
{
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL, *xmlrpc_temp = NULL;

	assert( server );

    env = va_arg( argv, xmlrpc_env * );
    char *file_path = va_arg( argv, char * );

	CLoadModuleInDevelopmentResult result;
	CError error = server->get_module_manager_writable().load_module_in_development( file_path, result );

	if( error != CError::SUCCESS )
	{
        free( file_path );
		return ntg_xmlrpc_error( env, error );
	}

    struct_ = xmlrpc_struct_new(env);

	string module_id_string = CGuidHelper::guid_to_string( result.module_id );

    xmlrpc_temp = xmlrpc_string_new(env, "module.loadmoduleindevelopment");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_temp = xmlrpc_string_new(env, module_id_string.c_str() );
    xmlrpc_struct_set_value(env, struct_, "moduleid", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

	if( result.previous_module_id != CGuidHelper::null_guid )
	{
		string previous_module_id_string = CGuidHelper::guid_to_string( result.previous_module_id );
		xmlrpc_temp = xmlrpc_string_new(env, previous_module_id_string.c_str() );
	    xmlrpc_struct_set_value(env, struct_, "previousmoduleid", xmlrpc_temp );
	    xmlrpc_DECREF( xmlrpc_temp );

		xmlrpc_temp = xmlrpc_bool_new(env, result.previous_remains_as_embedded );
	    xmlrpc_struct_set_value(env, struct_, "previousremainsasembedded", xmlrpc_temp );
	    xmlrpc_DECREF( xmlrpc_temp );
	}

    /* free out-of-place memory */
    free( file_path );

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_install_embedded_module_callback( CServer *server, const int argc, va_list argv )
{
    char *module_id_string;
	GUID module_id;
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL, *xmlrpc_temp = NULL;

	assert( server );

    env = va_arg( argv, xmlrpc_env * );
    module_id_string = va_arg( argv, char * );

	if( CGuidHelper::string_to_guid( module_id_string, module_id ) != CError::SUCCESS )
	{
        free( module_id_string );
		return ntg_xmlrpc_error( env, CError::INPUT_ERROR );
	}

    free( module_id_string );

	CError error = server->get_module_manager_writable().install_embedded_module( module_id );
	if( error != CError::SUCCESS )
	{
		return ntg_xmlrpc_error( env, error );
	}

    struct_ = xmlrpc_struct_new(env);
    xmlrpc_temp = xmlrpc_string_new(env, "module.installembeddedmodule");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_uninstall_module_callback( CServer *server, const int argc, va_list argv )
{
    char *module_id_string;
	GUID module_id;
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL, *xmlrpc_temp = NULL;

	assert( server );

    env = va_arg( argv, xmlrpc_env * );
    module_id_string = va_arg( argv, char * );

	if( CGuidHelper::string_to_guid( module_id_string, module_id ) != CError::SUCCESS )
	{
        free( module_id_string );
		return ntg_xmlrpc_error( env, CError::INPUT_ERROR );
	}

    free( module_id_string );

	CModuleUninstallResult result;
	CError error = server->get_module_manager_writable().uninstall_module( module_id, result );

	if( error != CError::SUCCESS )
	{
		return ntg_xmlrpc_error( env, error );
	}

    struct_ = xmlrpc_struct_new(env);
    xmlrpc_temp = xmlrpc_string_new(env, "module.uninstallmodule");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

	xmlrpc_temp = xmlrpc_bool_new( env, result.remains_as_embedded );
    xmlrpc_struct_set_value( env, struct_, "remainsasembedded", xmlrpc_temp );
    xmlrpc_DECREF( xmlrpc_temp );

	/* free out-of-place memory */

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_new_callback(CServer *server, const int argc, va_list argv )
{
    char *module_id_string, *node_name;
	GUID module_id;
    CPath *path;
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL, *xmlrpc_temp = NULL, *xmlrpc_path = NULL;

    env = va_arg(argv, xmlrpc_env *);
    module_id_string = va_arg(argv, char *);
    node_name = va_arg(argv, char *);
    path = va_arg(argv, CPath *);

	CGuidHelper::string_to_guid( module_id_string, module_id );

	CNewCommandResult result;
	CError error = server->process_command( CNewCommandApi::create( module_id, node_name, *path ), CCommandSource::XMLRPC_API, &result );

	const CNode *node = result.get_created_node();
	if( error != CError::SUCCESS || !node ) 
	{
        /* free out-of-place memory */
        free(module_id_string);
        free(node_name);
		return ntg_xmlrpc_error(env, error != CError::SUCCESS ? error : CError::INPUT_ERROR );
    }

    struct_ = xmlrpc_struct_new(env);

    xmlrpc_temp = xmlrpc_string_new(env, "command.new");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_temp = xmlrpc_string_new(env, module_id_string);
    xmlrpc_struct_set_value(env, struct_, "moduleid", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_temp = xmlrpc_string_new(env, node->get_name().c_str() );
    xmlrpc_struct_set_value(env, struct_, "instancename", xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_path = ntg_xmlrpc_value_from_path( *path, env);
    xmlrpc_struct_set_value(env, struct_, "path", xmlrpc_path);
    xmlrpc_DECREF(xmlrpc_path);

    /* free out-of-place memory */
    free(module_id_string);
    free(node_name);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_set_callback( CServer *server, const int argc, va_list argv )
{
    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL;
    CPath *path;
    CValue *value = NULL;
    xmlrpc_value *value_xmlrpc = NULL, *xmlrpc_temp = NULL, *xmlrpc_path;

    env = va_arg(argv, xmlrpc_env *);
    path = va_arg(argv, CPath *);
	value = va_arg(argv, CValue *);

    struct_ = xmlrpc_struct_new(env);

	INTEGRA_TRACE_VERBOSE << "setting value: " << path->get_string();

	CError error = server->process_command( CSetCommandApi::create( *path, value ), CCommandSource::XMLRPC_API );
	if( error != CError::SUCCESS )
	{
		return ntg_xmlrpc_error (env, error );
	}

    xmlrpc_temp = xmlrpc_string_new(env, "command.set");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_path = ntg_xmlrpc_value_from_path( *path, env);
    xmlrpc_struct_set_value(env, struct_, "path", xmlrpc_path);
    xmlrpc_DECREF(xmlrpc_path);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_get_callback( CServer *server, const int argc, va_list argv )
{

    xmlrpc_env *env;
    xmlrpc_value *struct_ = NULL;
    CPath *path;
    const CValue *value = NULL;
    xmlrpc_value *value_xmlrpc = NULL, *xmlrpc_temp = NULL, *xmlrpc_path;

    env = va_arg(argv, xmlrpc_env *);
    path = va_arg(argv, CPath *);

    struct_ = xmlrpc_struct_new(env);

	INTEGRA_TRACE_VERBOSE << "getting value: " << path->get_string();

    value = server->get_value( *path );

	if( value )
	{
		value_xmlrpc = ntg_xmlrpc_value_new( *value, env );
	}
	else
	{
		value_xmlrpc = xmlrpc_nil_new( env );
	}
 
    xmlrpc_temp = xmlrpc_string_new(env, "query.get");
    xmlrpc_struct_set_value(env, struct_, RESPONSE_LABEL, xmlrpc_temp);
    xmlrpc_DECREF(xmlrpc_temp);

    xmlrpc_path = ntg_xmlrpc_value_from_path( *path, env);
    xmlrpc_struct_set_value(env, struct_, "path", xmlrpc_path);
    xmlrpc_DECREF(xmlrpc_path);

    xmlrpc_struct_set_value(env, struct_, "value", value_xmlrpc);
    xmlrpc_DECREF(value_xmlrpc);

    return struct_;
}


static xmlrpc_value *ntg_xmlrpc_version(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    INTEGRA_TRACE_VERBOSE;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_version_callback, server, 1, (void *)env);
}

static xmlrpc_value *ntg_xmlrpc_print_state(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    INTEGRA_TRACE_VERBOSE;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_print_state_callback, server, 1, (void *)env);
}


static xmlrpc_value *ntg_xmlrpc_interfacelist(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    INTEGRA_TRACE_VERBOSE;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_interfacelist_callback, server, 1, (void *)env);
}


static xmlrpc_value *ntg_xmlrpc_interfaceinfo(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    const char *name;
    size_t len;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(s#)", &name, &len);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_interfaceinfo_callback, server, 2, (void *)env, name);
}


static xmlrpc_value *ntg_xmlrpc_endpoints(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    const char *name;
    size_t len;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(s#)", &name, &len);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_endpoints_callback, server, 2, (void *)env, name);
}


static xmlrpc_value *ntg_xmlrpc_widgets(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    const char *name;
    size_t len;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(s#)", &name, &len);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_widgets_callback, server, 2, (void *)env, name);
}


static xmlrpc_value *ntg_xmlrpc_default(xmlrpc_env * const env,
        const char *const host,
        const char *const method_name,
        xmlrpc_value * const param_array,
        void *const user_data)
{
    INTEGRA_TRACE_ERROR << "WARNING, unhandled method: " << method_name;

    return param_array;
}


static xmlrpc_value *ntg_xmlrpc_nodelist(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    xmlrpc_value *xmlrpc_path;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(A)", &xmlrpc_path);

    CPath path = ntg_xmlrpc_get_path(env, xmlrpc_path);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_nodelist_callback, server, 2, (void *)env, &path);

}

static xmlrpc_value *ntg_xmlrpc_delete(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    xmlrpc_value *xmlrpc_path;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(A)", &xmlrpc_path);
    CPath path = ntg_xmlrpc_get_path(env, xmlrpc_path);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_delete_callback, server, 2, (void *)env, &path);

}

static xmlrpc_value *ntg_xmlrpc_rename(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    xmlrpc_value *xmlrpc_path;
    const char *name;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(As)", &xmlrpc_path, &name);
    CPath path = ntg_xmlrpc_get_path(env, xmlrpc_path);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_rename_callback, server, 3, (void *)env, &path, name);

}

static xmlrpc_value *ntg_xmlrpc_save(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    xmlrpc_value *xmlrpc_path;
    const char *file_path;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(As)", &xmlrpc_path, &file_path);
    CPath path = ntg_xmlrpc_get_path(env, xmlrpc_path);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_save_callback, server, 3, (void *)env, &path, file_path);
}

static xmlrpc_value *ntg_xmlrpc_load(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    xmlrpc_value *xmlrpc_path;
    const char *file_path;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(sA)", &file_path, &xmlrpc_path);
    CPath path = ntg_xmlrpc_get_path(env, xmlrpc_path);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_load_callback, server, 3, (void *)env, file_path, &path);

}

static xmlrpc_value *ntg_xmlrpc_move(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    xmlrpc_value *xmlrpc_node_path, *xmlrpc_parent_path;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(AA)", &xmlrpc_node_path,
            &xmlrpc_parent_path);

    CPath node_path = ntg_xmlrpc_get_path(env, xmlrpc_node_path);
    CPath parent_path = ntg_xmlrpc_get_path(env, xmlrpc_parent_path);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_move_callback, server, 3, (void *)env, &node_path, &parent_path);

}

static xmlrpc_value *ntg_xmlrpc_unload_orphaned_embedded(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    INTEGRA_TRACE_VERBOSE;

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_unload_orphaned_embedded_callback, server, 1, (void *)env );
}


static xmlrpc_value *ntg_xmlrpc_install_module_file(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    const char *file_name;
    size_t len;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(s#)", &file_name, &len);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_install_module_callback, server, 2, (void *)env, file_name );
}


static xmlrpc_value *ntg_xmlrpc_install_embedded_module(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    const char *module_id_string;
    size_t len;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(s#)", &module_id_string, &len);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_install_embedded_module_callback, server, 2, (void *)env, module_id_string );
}


static xmlrpc_value *ntg_xmlrpc_uninstall_module(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    const char *module_id_string;
    size_t len;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(s#)", &module_id_string, &len);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_uninstall_module_callback, server, 2, (void *)env, module_id_string );
}


static xmlrpc_value *ntg_xmlrpc_load_module_in_development(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    const char *file_name;
    size_t len;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(s#)", &file_name, &len);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_load_module_in_development_callback, server, 2, (void *)env, file_name );
}




static xmlrpc_value *ntg_xmlrpc_new(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{

    const char *name, *node_name;
    xmlrpc_value *xmlrpc_path;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(ssA)", &name, &node_name,
            &xmlrpc_path);

    CPath path = ntg_xmlrpc_get_path(env, xmlrpc_path);

    xmlrpc_DECREF(xmlrpc_path);

    if (env->fault_occurred)
        return NULL;

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_new_callback, server, 4, (void *)env, name, node_name, &path );
}


static xmlrpc_value *ntg_xmlrpc_set(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{
    xmlrpc_value *xmlrpc_value_, *xmlrpc_path;
    xmlrpc_value *xmlrpc_rv;
    CValue *value;
	int number_of_elements;

    INTEGRA_TRACE_VERBOSE;

	number_of_elements = xmlrpc_array_size( env, parameter_array );
	switch( number_of_elements )
	{
		case 1:
			xmlrpc_decompose_value(env, parameter_array, "(A)", &xmlrpc_path );
			xmlrpc_value_ = NULL;
			value = NULL;
			break;

		case 2:
			xmlrpc_decompose_value(env, parameter_array, "(AV)", &xmlrpc_path, &xmlrpc_value_ );
		    value = ntg_xmlrpc_get_value( env, xmlrpc_value_ );
			break;

		default:
			INTEGRA_TRACE_ERROR << "incorrect number of parameters passed into xmlrpc command.set: " << number_of_elements;
			return NULL;
	}

    CPath path = ntg_xmlrpc_get_path(env, xmlrpc_path);

	CServer *server = ( CServer * ) user_data;
	xmlrpc_rv = ntg_server_do_va( &ntg_xmlrpc_set_callback, server, 3, (void *)env, &path, value );

	delete value;

    return xmlrpc_rv;
}

static xmlrpc_value *ntg_xmlrpc_get(xmlrpc_env * const env,
        xmlrpc_value *parameter_array,
        void *user_data)
{

    xmlrpc_value *xmlrpc_path;

    INTEGRA_TRACE_VERBOSE;

    xmlrpc_decompose_value(env, parameter_array, "(A)", &xmlrpc_path);

    CPath path = ntg_xmlrpc_get_path(env, xmlrpc_path);

    xmlrpc_DECREF(xmlrpc_path);

	CServer *server = ( CServer * ) user_data;
    return ntg_server_do_va(&ntg_xmlrpc_get_callback, server, 2, (void *)env, &path);

}


void ntg_xmlrpc_shutdown( xmlrpc_env *const envP, 
        void *       const context,
        const char * const comment,
        void *       const callInfo)
{
	CServer *server = ( CServer * ) context;
	server->send_shutdown_signal();
}


void *ntg_xmlrpc_server_run( void *context )
{
    xmlrpc_registry *registryP;
    xmlrpc_env env;

	CXmlRpcServerContext *server_context = ( CXmlRpcServerContext * ) context;
	unsigned short port = server_context->m_port;
	sem_t *sem_initialized = server_context->m_sem_initialized;

	CServer *server = server_context->m_server;
	delete server_context;

    INTEGRA_TRACE_PROGRESS << "Starting server on port " << port;

    xmlrpc_env_init(&env);

    registryP = xmlrpc_registry_new(&env);

    xmlrpc_limit_set( XMLRPC_XML_SIZE_LIMIT_ID, NTG_XMLRPC_SIZE_LIMIT );

    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "system.version",
            &ntg_xmlrpc_version, server,
            "S:", HELPSTR_VERSION);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "system.printstate",
            &ntg_xmlrpc_print_state, server,
            "S:", HELPSTR_PRINTSTATE);
	xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "query.interfacelist",
            &ntg_xmlrpc_interfacelist, server, "S:",
            HELPSTR_INTERFACELIST);
	xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "query.interfaceinfo",
            &ntg_xmlrpc_interfaceinfo, server, "S:s",
            HELPSTR_INTERFACEINFO);
	xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "query.endpoints",
            &ntg_xmlrpc_endpoints, server, "S:s",
            HELPSTR_ENDPOINTS);
	xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "query.widgets",
            &ntg_xmlrpc_widgets, server, "S:s",
            HELPSTR_WIDGETS);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "query.nodelist",
            &ntg_xmlrpc_nodelist, server, "S:A",
            HELPSTR_NODELIST);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "query.get",
            &ntg_xmlrpc_get, server, "S:A",
            HELPSTR_GET);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "command.set",
            &ntg_xmlrpc_set, server,
            "S:As,S:Ai,S:Ad,S:A6", HELPSTR_SET);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "command.new",
            &ntg_xmlrpc_new, server, "S:ssA",
            HELPSTR_NEW);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "command.delete",
            &ntg_xmlrpc_delete, server, "S:A",
            HELPSTR_DELETE);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "command.rename",
            &ntg_xmlrpc_rename, server, "S:As",
            HELPSTR_RENAME);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "command.save",
            &ntg_xmlrpc_save, server, "S:As",
            HELPSTR_SAVE);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "command.load",
            &ntg_xmlrpc_load, server, "S:sA",
            HELPSTR_LOAD);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "command.move",
            &ntg_xmlrpc_move, server, "S:AA",
            HELPSTR_MOVE);

    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "module.unloadorphanedembedded",
            &ntg_xmlrpc_unload_orphaned_embedded, server, "S:",
            HELPSTR_UNLOAD_ORPHANED_EMBEDDED);
	xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "module.installintegramodulefile",
        &ntg_xmlrpc_install_module_file, server, "S:s",
        HELPSTR_INSTALL_INTEGRA_MODULE_FILE);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "module.installembeddedmodule",
        &ntg_xmlrpc_install_embedded_module, server, "S:s",
        HELPSTR_INSTALL_EMBEDDED_MODULE);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "module.uninstallmodule",
        &ntg_xmlrpc_uninstall_module, server, "S:s",
        HELPSTR_UNINSTALL_MODULE);
    xmlrpc_registry_add_method_w_doc(&env, registryP, NULL, "module.loadmoduleindevelopment",
        &ntg_xmlrpc_load_module_in_development, server, "S:s",
		HELPSTR_LOAD_MODULE_IN_DEVELOPMENT);

    xmlrpc_registry_set_shutdown(registryP, ntg_xmlrpc_shutdown, server );

    xmlrpc_registry_set_default_method(&env, registryP, &ntg_xmlrpc_default,
            NULL);

    ServerCreate(&abyssServer, "XmlRpcServer", port, NULL, 
            NULL);

    xmlrpc_server_abyss_set_handlers2(&abyssServer, "/", registryP);

    ServerInit(&abyssServer);

	sem_post( sem_initialized );

    /* This version processes commands concurrently */
#if 1
    ServerRun(&abyssServer);
#else
    while (true){
        ServerRunOnce(&abyssServer);
        /* This waits for the next connection, accepts it, reads the
           HTTP POST request, executes the indicated RPC, and closes
           the connection.
           */
    }
#endif

    ServerFree(&abyssServer);
    xmlrpc_registry_free(registryP);

    /* FIX: for now we support the old 'stable' version of xmlrpc-c */
    /* AbyssTerm(); */

    xmlrpc_env_clean(&env);

    INTEGRA_TRACE_PROGRESS << "XMLRPC server terminated";
    pthread_exit(0);

    return NULL;

}

void ntg_xmlrpc_server_terminate( sem_t *sem_initialized )
{
    /* make sure we don't try to terminate before abyssServer init */
    sem_wait( sem_initialized );
    ServerTerminate(&abyssServer);
}

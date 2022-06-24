/*
 * WriteThruShellModule.cc
 *
 * Simple WriteThru shell
 * Copyright (c) 2008, 2020 Linas Vepstas <linas@linas.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <opencog/util/Logger.h>
#include <opencog/util/oc_assert.h>
#include <opencog/cogserver/server/ServerConsole.h>

#include "WriteThruShell.h"
#include "WriteThruShellModule.h"

using namespace opencog;

DECLARE_MODULE(WriteThruShellModule);

WriteThruShellModule::WriteThruShellModule(CogServer& cs) : Module(cs)
{
}

void WriteThruShellModule::init(void)
{
	_cogserver.registerRequest(shelloutRequest::info().id,
	                           &shelloutFactory);
}

WriteThruShellModule::~WriteThruShellModule()
{
	_cogserver.unregisterRequest(shelloutRequest::info().id);
}

const RequestClassInfo&
WriteThruShellModule::shelloutRequest::info(void)
{
	static const RequestClassInfo _cci("wthru",
		"Enter the write-through s-expression shell",
		"Usage: wthru\n\n"
		"Enter the write-through s-expression interpreter shell. This shell\n"
		"is exactly like the s-expression shell (sexpr), except that it\n"
		"provides write-through services.  This is not intended for manual\n"
		"use; its for StorageNode communications. Say `jelp sexpr` for more\n"
		"info.\n\n"
		"If you enter the shell by accident, then us either a ^D (ctrl-D) or\n"
		" a single . on a line by itself to exit the shell.\n",
		true, false);
	return _cci;
}

/**
 * Register this shell with the console.
 */
bool
WriteThruShellModule::shelloutRequest::execute(void)
{
	ServerConsole *con = this->get_console();
	OC_ASSERT(con, "Invalid Request object");

	WriteThruShell *sh = new WriteThruShell();
	sh->set_socket(con);

	send("");
	return true;
}

/* ===================== END OF FILE ============================ */

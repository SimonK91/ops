/**
*
* Copyright (C) 2018-2020 Lennart Andersson.
*
* This file is part of OPS (Open Publish Subscribe).
*
* OPS (Open Publish Subscribe) is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* OPS (Open Publish Subscribe) is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with OPS (Open Publish Subscribe).  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include "IOService.h"

class RAII_ioServ
{
public:
	std::unique_ptr<ops::IOService> ioServ;
	RAII_ioServ() : ioServ(ops::IOService::create()) { }
	~RAII_ioServ() { }
	ops::IOService* operator()() { return ioServ.get(); }
	RAII_ioServ(RAII_ioServ const &) = delete;
	RAII_ioServ(RAII_ioServ&&) = delete;
	RAII_ioServ& operator=(RAII_ioServ&&) = delete;
	RAII_ioServ& operator=(RAII_ioServ const&) = delete;
};

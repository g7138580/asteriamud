function ShowMemory( unit )
	Send( unit, string.format( "Lua Memory          : %.2fMB\r\n", collectgarbage( "count" ) / 1024 ) )
end

function SetRoomVar( data, var, val )
	local room = Rooms[data]

	if room == nil then
		NewRoom( data )
		room = Rooms[data]
	end

	if room.variables == nil then
		room.variables = {}
	end

	room.variables[var] = val
end

function GetRoomVar( data, var )
	local room = Rooms[data]

	if room == nil then
		return nil
	end

	if room.variables == nil then
		return nil
	end

	return room.variables[var]
end

function SetRoomTimer( room, count, func, ... )
	table.insert( RoomTimers, { room, count, func, arg } )
end

Log( "\t\tbase_functions" )

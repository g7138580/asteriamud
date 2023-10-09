math.randomseed( os.time() )
math.random()
math.random()
math.random()

local Triggers = {}
local threads = {}

Rooms = {}
Units = {}

RoomTimers = {}

north = 0
east = 1
south = 2
west = 3
northeast = 4; ne = 4
southeast = 5; se = 5
southwest = 6; sw = 6
northwest = 7; nw = 7
up = 8
down = 9

ACT_SELF = 1
ACT_TARGET = 2
ACT_OTHERS = 4
ACT_HIDDEN = 8
ACT_CANT_SEE = 16
ACT_NO_COLOR = 32
ACT_FOLLOW = 64
ACT_LAST = 128
ACT_WRAP = 256
ACT_SOCIAL = 512

COMMAND_NOT_FOUND = 2

function CompileTrigger( data, text )
	if ( data == nil or text == nil ) then
		return
	end

	Triggers[data] = loadstring( "local self, ch, room, obj, arg = ...\n\n" .. text )
end

function DestroyTrigger( data )
	Triggers[data] = nil
end

function NewRoom( data )
	Rooms[data] = {}
end

function DeleteRoom( data )
	Rooms[data] = nil
end

function NewUnit( data )
	Units[data] = {}
end

function DeleteUnit( data )
	Units[data] = nil
end

function WaitUpdate()
	local t = threads

	for i = #RoomTimers, 1, -1 do
		local r = RoomTimers[i]

		if ( r ~= nil ) then
			r[2] = r[2] - 1
			if ( r[2] <= 0 ) then
				r[3]( r[4] )
				RoomTimers[i] = nil
			end
		end
	end

	for k, v in pairs( t ) do
		if ( os.time() >= v.time ) then
			t[k] = nil
			assert( coroutine.resume( k ) )
		end
	end
end

function Delay( seconds )
	threads[coroutine.running()] = {}
	threads[coroutine.running()].time = os.time() + ( seconds or 1 )

	return coroutine.yield()
end

function Run( trigger, self, ch, room, obj, arg )
	local func = Triggers[trigger]

	assert( type( func ) == "function", "wait.make requires a function" )

	local run = coroutine.wrap( func )( self, ch, room, obj, arg )

	return run
end

Log( "\t\tstartup" )

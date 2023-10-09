package.path = package.path .. ";../scripts/?.lua"

-- set random seed and flush out any residual random numbers
math.randomseed( os.time() )
math.random()
math.random()
math.random()

local Triggers = {}

_MOBILE = {}
_ROOM = {}
_ITEM = {}

ACT_NOONE = 1
ACT_SELF = 2
ACT_TARGET = 4
ACT_OTHERS = 8
ACT_HIDDEN = 16
ACT_NOCOLOR = 32
ACT_FORMAT = 64
ACT_NOTFOLLOW = 128
ACT_SPELLCRAFT = 256

ACT_COMBAT = ACT_SELF + ACT_TARGET + ACT_OTHERS
ACT_ALL = ACT_SELF + ACT_TARGET + ACT_OTHERS
ACT_ALL_CANSEE = ACT_ALL + ACT_HIDDEN
ACT_OTHERS_CANSEE = ACT_OTHERS + ACT_HIDDEN

COMMAND_NOT_FOUND = 2

_MOBILE_TIMER = {}
_ROOM_TIMER = {}
_ITEM_TIMER = {}

----------------
-- DIRECTIONS
----------------
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

require( "BasicFunctions" )

local threads = {}

local preCompile =
"local self, ch, room, obj, arg = ...\n\n"

function CompileTrigger( userData, text )

	if ( userData == nil or text == nil ) then
		return
	end

	Triggers[userData] = loadstring( preCompile .. text )

	return

end

function DestroyTrigger( userData, UID )

	Triggers[userData] = nil

	return

end

function NewRoom( ID )

	_ROOM[ID] = {}

	return

end

function NewUnit( ID )

	_MOBILE[ID] = {}

	return

end

function NewItem( ID )

	_ITEM[ID] = {}

	return

end

function DeleteItem( ID )

	_ITEM[ID] = nil

	return

end

function DeleteRoom( ID )

	_ROOM[ID] = nil

	return

end

function DeleteUnit( ID )

	_MOBILE[ID] = nil

	return

end

local CollectGarbage = 0

function WaitUpdate()

	for i = #_MOBILE_TIMER, 1, -1 do
		local r = _MOBILE_TIMER[i]

		if ( r ~= nil ) then
			r[2] = r[2] - 1

			if r[2] <= 0 then
				r[3](r[4])
				_MOBILE_TIMER[i] = nil
			end
		end
	end

	for i = #_ROOM_TIMER, 1, -1 do
		local r = _ROOM_TIMER[i]

		if ( r ~= nil ) then
			r[2] = r[2] - 1

			if r[2] <= 0 then
				r[3](r[4])
				_ROOM_TIMER[i] = nil
			end
		end
	end

	for i = #_ITEM_TIMER, 1, -1 do
		local r = _ITEM_TIMER[i]

		if ( r ~= nil ) then
			r[2] = r[2] - 1

			if r[2] <= 0 then
				r[3](r[4])
				_ITEM_TIMER[i] = nil
			end
		end
	end

	for k, v in pairs( threads ) do
		if ( os.time() >= v.time ) then
			threads[k] = nil
			assert( coroutine.resume( k ) )
		end
	end

	CollectGarbage = CollectGarbage + 1

	if CollectGarbage > 100 then
		collectgarbage( "collect" )
		CollectGarbage = 0
	end

	return

end

function Delay( seconds )

	threads[coroutine.running()] = {}
	threads[coroutine.running()].time = os.time() + ( seconds or 1 )

	return coroutine.yield()

end

function ShowMemory( ID )

	local mem = string.format( "%.2f", collectgarbage( "count" ) / 1024 )

	SendMobile( ID, "\n\rLua Memory: " .. mem .. "MB" )

	return

end

--local preCompile =
--"local selfData, chData, roomData, objData, arg = ...\n" ..
--"local self = MobileClass:New( GetClass( selfData, Mobiles ) )\n" ..
--"local ch = MobileClass:New( GetClass( chData, Mobiles ) )\n" ..
--"local room = RoomClass:New( GetClass( roomData, Rooms ) )\n" ..
--"local obj = ObjClass:New( GetClass( objData, Objects ) )\n\n"

function Run( triggerData, self, ch, room, obj, arg )

	local f = Triggers[triggerData]

	assert( type( f ) == "function", "wait.make requires a function" )

    local test = coroutine.wrap( f )( self, ch, room, obj, arg )  -- make coroutine, resume it

	return test

end

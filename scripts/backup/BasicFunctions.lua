function IsRoomIncluded( room, ... )

	for i = 1, #arg do
		if room == arg[i] then return true end
	end

	return false

end

function IsRoomExcluded( room, ... )

	for i = 1, #arg do
		if room == arg[i] then return true end
	end

	return false

end

function Log( message ) cfunc.Log( message ); return; end
function Echo( message ) cfunc.Echo( message ); return; end
function SendMobile( UID, text ) cfunc.SendMobile( UID, text .. "^n\r\n" ); return; end

function MoveMobile( UID, dir, bScript )

	if bScript then
		cfunc.MoveMobile( UID, dir, 1 )
	else
		cfunc.MoveMobile( UID, dir, 0 )
	end

	return

end

function Transfer( UID, vnum )

	cfunc.Transfer( UID, vnum )

	return

end

function Open( UID, dir )

	cfunc.Open( UID, dir )

	return

end

function GetGuild( UID )

	return cfunc.GetGuild( UID )

end

function GetGuildRank( UID )

	return cfunc.GetGuildRank( UID )

end

function GetGuildName( UID )

	local name = cfunc.GetGuildName( UID )

	if name == 0 then
		return "No Guild"
	else
		return name
	end

end

function GetGuildRankName( UID )

	local name = cfunc.GetGuildRankName( UID )

	if name == 0 then
		return "No Rank"
	else
		return name
	end

end

function QuestCompleted( UID, vnum )

	return cfunc.QuestCompleted( UID, vnum )

end

function MobileHasItem( UID, vnum )

	return cfunc.MobileHasItem( UID, vnum )

end

function MobileCarryingItem( UID, vnum )

	return cfunc.MobileCarryingItem( UID, vnum )

end

function CreateItem( vnum )

	return cfunc.CreateItem( vnum )

end

function DestroyItem( UID )

	cfunc.DestroyItem( UID )

end

function GetItemFromMobile( UID, vnum )

	return cfunc.GetItemFromMobile( UID, vnum )

end

function GiveItem( mobileUID, itemUID )

	return cfunc.GiveItem( mobileUID, itemUID )

end

function TakeItem( UID, vnum )

	cfunc.TakeItem( UID, vnum )

	return

end

function RoomHasItem( UID, vnum )

	return cfunc.RoomHasItem( UID, vnum )

end

function GetLevel( UID )

	return cfunc.GetLevel( UID )

end

function GetBalance( UID )

	return cfunc.GetBalance( UID )

end

function SetBalance( UID, value )

	cfunc.SetBalance( UID, value )

	return

end

function SetEnemies( UID, UID_Target )

	cfunc.SetEnemies( UID, UID_Target )

	return

end

function Act( UID, flags, text )

	cfunc.Act( UID, flags + ACT_FORMAT, text .. "&n\n\r" )

	return

end

function GetGoldMobile( UID )

	return cfunc.GetGoldMobile( UID )

end

function GetGoldRoom( UID )

	return cfunc.GetGoldRoom( UID )

end

function GiveGold( UID, gold )

	cfunc.GiveGold( UID, gold )

	return

end

function TakeGold( UID, gold )

	cfunc.TakeGold( UID, gold )

	return

end

function DropGold( UID, gold )

	cfunc.DropGold( UID, gold )

	return

end

function DropItem( UID, vnum )

	cfunc.DropItem( UID, vnum )

	return

end

function AddEffectMobile( UID, vnum )

	cfunc.AddEffectMobile( UID, vnum )

	return

end

function RemoveEffectMobile( UID, vnum )

	cfunc.RemoveEffectMobile( UID, vnum )

	return

end

function MobileHasEffect( UID, vnum )

	return cfunc.MobileHasEffect( UID, vnum )

end

function GetBloodline( UID )

	return cfunc.GetBloodline( UID )

end

function SetBloodline( UID, arg )

	cfunc.SetBloodline( UID, arg )

	return

end

function CompletedQuest( UID, vnum )

	return cfunc.CompletedQuest( UID, vnum )

end

function SendRoom( UID, message )

	cfunc.SendRoom( UID, message .. "&n\n\r" )

	return

end

function GetMobilesInRoom( UID )

	return cfunc.GetMobilesInRoom( UID )

end

function AddMonsterToRoom( UID, vnum )

	cfunc.AddMonsterToRoom( UID, vnum )

	return

end

function SetRoomVariable( UID, var, value )

	local R = _ROOM[UID]

	if R == nil then return end

	if R.Variables == nil then R.Variables = {} end

	R.Variables[var] = value

	return

end

function GetRoomVariable( UID, var )

	local R = _ROOM[UID]

	if R == nil then return nil end

	if R.Variables == nil then return nil end

	return R.Variables[var]

end

function SetRoomTimer( UID, count, func, ... )

	table.insert( _ROOM_TIMER, { UID, count, func, arg } )

	return

end

function SetMobileVariable( UID, var, value )

	local M = _MOBILE[UID]

	if M == nil then return end

	if M.Variables == nil then M.Variables = {} end

	M.Variables[var] = value

	return

end

function GetMobileVariable( UID, var )

	local M = _MOBILE[UID]

	if M == nil then return nil end

	if M.Variables == nil then return nil end

	return M.Variables[var]

end

function SetMobileTimer( UID, count, func, ... )

	table.insert( _MOBILE_TIMER, { UID, count, func, arg } )

	return

end

function EntryCreatesItem( ch, room, vnum )

	if MobileHasItem( ch, vnum ) or RoomHasItem( room, vnum ) then return false end

	DropItem( room, vnum )

	return true
end

function Restore( UID )

	cfunc.Restore( UID )

	return
end

function Kill( attackerUID, targetUID )

	cfunc.Kill( attackerUID, targetUID )

	return

end

function MonsterExists( vnum )

	return cfunc.MonsterExists( vnum )

end

function IsFlying( UID )

	return cfunc.IsFlying( UID )

end

function Purge( UID )

	cfunc.Purge( UID )

	return

end

function GetMonsterInRoom( room, vnum )

	return cfunc.GetMonsterInRoom( room, vnum )

end

function GetHealth( UID )

	return cfunc.GetHealth( UID )

end

function GetMaxHealth( UID )

	return cfunc.GetMaxHealth( UID )

end

function GetMana( UID )

	return cfunc.GetMana( UID )

end

function GetMaxMana( UID )

	return cfunc.GetMaxMana( UID )

end

function InCombat( UID ) return cfunc.InCombat( UID ) end
function GetGold( UID ) return cfunc.GetGold( UID ) end
function Freeze( UID ) return cfunc.Freeze( UID ) end
function Unfreeze( UID ) return cfunc.Unfreeze( uid ) end
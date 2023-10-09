#include "Commands/Command.h"

const COMMAND CommandTable[] =
{
	{	"'",								cmdSay,				0,															TRUST_PLAYER		},

	{	"attack",							cmdAttack,			CMD_BALANCED | CMD_HIDDEN,									TRUST_PLAYER		},

	{	"accept",							cmdAccept,			0,															TRUST_PLAYER		},
	{	"activate",							cmdActivate,		CMD_BALANCED,												TRUST_PLAYER		},
	{	"affected",							cmdAffected,		0,															TRUST_PLAYER		},
	{	"afk",								cmdAFK,				0,															TRUST_PLAYER		},
	{	"appraise",							cmdAppraise,		CMD_BALANCED,												TRUST_PLAYER		},
	{	"armor",							cmdArmor,			0,															TRUST_PLAYER		},

	{	"bounty",							cmdBounty,			0,															TRUST_PLAYER		},
	{	"brew",								cmdBrew,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"buy",								cmdBuy,				CMD_EXACT | CMD_BALANCED,									TRUST_PLAYER		},
	{	"buyback",							cmdBuyback,			CMD_EXACT | CMD_BALANCED,									TRUST_PLAYER		},

	{	"bug",								cmdBug,				CMD_EXACT,													TRUST_PLAYER		},

	{	"cast",								cmdCast,			CMD_BALANCED | CMD_HIDDEN,									TRUST_PLAYER		},
	{	"craft",							cmdCraft,			CMD_BALANCED,												TRUST_PLAYER		},

	{	"channels",							cmdChannels,		0,															TRUST_PLAYER		},
	{	"changes",							cmdChanges,			0,															TRUST_PLAYER		},
	{	"chat",								cmdChat,			0,															TRUST_PLAYER		},
	{	"clean",							cmdClean,			CMD_EXACT | CMD_BALANCED,									TRUST_PLAYER		},
	{	"chop",								cmdChop,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"cook",								cmdCook,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"colors",							cmdColors,			0,															TRUST_PLAYER		},
	{	"combatprompt",						cmdCombatPrompt,	0,															TRUST_PLAYER		},
	{	"commands",							cmdCommands,		0,															TRUST_PLAYER		},
	{	"consider",							cmdConsider,		0,															TRUST_PLAYER		},
	{	"config",							cmdConfig,			0,															TRUST_PLAYER		},
	{	"cprompt",							cmdCombatPrompt,	0,															TRUST_PLAYER		},
	{	"craft",							cmdCraft,			CMD_BALANCED,												TRUST_PLAYER		},

	{	"copyover",							cmdCopyover,		CMD_EXACT,													TRUST_ADMIN			},

	{	"defenses",							cmdDefenses,		0,															TRUST_PLAYER		},
	{	"defend",							cmdDefend,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"deposit",							cmdDeposit,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"describe",							cmdDescribe,		CMD_BALANCED,												TRUST_PLAYER		},
	{	"description",						cmdDescription,		CMD_BALANCED,												TRUST_PLAYER		},
	{	"destroy",							cmdDestroy,			CMD_EXACT | CMD_BALANCED,									TRUST_PLAYER		},
	{	"die",								cmdDie,				CMD_EXACT,													TRUST_PLAYER		},
	{	"disenchant",						cmdDisenchant,		CMD_EXACT | CMD_BALANCED,									TRUST_PLAYER		},
	{	"dismiss",							cmdDismiss,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"dismount",							cmdDismount,		CMD_BALANCED,												TRUST_PLAYER		},
	{	"drink",							cmdDrink,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"drop",								cmdDrop,			CMD_BALANCED,												TRUST_PLAYER		},

	{	"dsave",							cmdDSave,			CMD_EXACT,													TRUST_ADMIN			},

	{	"eat",								cmdEat,				CMD_BALANCED,												TRUST_PLAYER		},
	{	"emote",							cmdEmote,			0,															TRUST_PLAYER		},
	{	"equipment",						cmdEquipment,		0,															TRUST_PLAYER		},
	{	"exits",							cmdExits,			0,															TRUST_PLAYER		},
	{	"experience",						cmdExperience,		0,															TRUST_PLAYER		},

	{	"fish",								cmdFish,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"follow",							cmdFollow,			0,															TRUST_PLAYER		},
	{	"forage",							cmdForage,			CMD_BALANCED,												TRUST_PLAYER		},

	{	"feedback",							cmdFeedback,		0,															TRUST_STAFF			},
	{	"force",							cmdForce,			0,															TRUST_STAFF			},

	{	"get",								cmdGet,				CMD_BALANCED,												TRUST_PLAYER		},
	{	"glance",							cmdGlance,			0,															TRUST_PLAYER		},
	{	"group",							cmdGroup,			0,															TRUST_PLAYER		},

	{	"gold",								cmdGold,			0,															TRUST_PLAYER		},
	{	"guildinfo",						cmdGuildInfo,		0,															TRUST_PLAYER		},
	{	"ginfo",							cmdGuildInfo,		0,															TRUST_PLAYER		},
	{	"guildmotd",						cmdGuildMOTD,		0,															TRUST_PLAYER		},
	{	"gmotd",							cmdGuildMOTD,		0,															TRUST_PLAYER		},
	{	"guildroster",						cmdGuildRoster,		0,															TRUST_PLAYER		},
	{	"groster",							cmdGuildRoster,		0,															TRUST_PLAYER		},
	{	"guildstats",						cmdGuildStats,		0,															TRUST_PLAYER		},
	{	"gs",								cmdGuildStats,		0,															TRUST_PLAYER		},
	{	"guildshout",						cmdGuildShout,		0,															TRUST_PLAYER		},
	{	"gshout",							cmdGuildShout,		0,															TRUST_PLAYER		},
	{	"guildchat",						cmdGuildChat,		0,															TRUST_PLAYER		},
	{	"gchat",							cmdGuildChat,		0,															TRUST_PLAYER		},
	{	"guildwho",							cmdGuildWho,		0,															TRUST_PLAYER		},
	{	"gwho",								cmdGuildWho,		0,															TRUST_PLAYER		},

	{	"gamesettings",						cmdGameSettings,	0,															TRUST_ADMIN			},
	{	"goto",								cmdGoto,			0,															TRUST_STAFF			},

	{	"hands",							cmdHands,			0,															TRUST_PLAYER		},
	{	"health",							cmdHealth,			0,															TRUST_PLAYER		},
	{	"hold",								cmdHold,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"help",								cmdHelp,			0,															TRUST_PLAYER		},

	{	"hedit",							cmdHedit,			0,															TRUST_STAFF			},
	{	"hfind",							cmdHelpFind,		0,															TRUST_STAFF			},

	{	"inventory",						cmdInventory,		0,															TRUST_PLAYER		},
	{	"idea",								cmdIdea,			CMD_EXACT,													TRUST_PLAYER		},

	{	"iload",							cmdItemLoad,		0,															TRUST_STAFF			},
	{	"immchat",							cmdImmChat,			0,															TRUST_STAFF			},
	{	"itemload",							cmdItemLoad,		0,															TRUST_STAFF			},

	{	"jewelry",							cmdJewelry,			0,															TRUST_PLAYER		},
	{	"journal",							cmdJournal,			0,															TRUST_PLAYER		},

	{	"killlist",							cmdKillList,		0,															TRUST_PLAYER		},
	{	"klist",							cmdKillList,		0,															TRUST_PLAYER		},

	{	"look",								cmdLook,			0,															TRUST_PLAYER		},

	{	"last",								cmdLast,			0,															TRUST_PLAYER		},
	{	"list",								cmdList,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"lose",								cmdLose,			0,															TRUST_PLAYER		},

	{	"laws",								cmdLaws,			0,															TRUST_STAFF			},
	{	"ledit",							cmdLedit,			0,															TRUST_STAFF			},
	{	"lfind",							cmdLootFind,		0,															TRUST_STAFF			},

	{	"macros",							cmdMacros,			0,															TRUST_PLAYER		},
	{	"mana",								cmdMana,			0,															TRUST_PLAYER		},
	{	"mine",								cmdMine,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"mount",							cmdMount,			CMD_BALANCED,												TRUST_PLAYER		},

	{	"memory",							cmdMemory,			0,															TRUST_ADMIN			},
	{	"mload",							cmdMonsterLoad,		0,															TRUST_STAFF			},
	{	"mset",								cmdMSet,			0,															TRUST_STAFF			},
	{	"mstat",							cmdMStat,			0,															TRUST_STAFF			},

	{	"next",								cmdNext,			0,															TRUST_PLAYER		},
	{	"newbiechat",						cmdNewbieChat,		0,															TRUST_PLAYER		},
	{	"news",								cmdNews,			0,															TRUST_PLAYER		},

	{	"offer",							cmdOffer,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"open",								cmdOpen,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"oreply",							cmdReply,			0,															TRUST_PLAYER		},
	{	"otell",							cmdTell,			0,															TRUST_PLAYER		},

	{	"olc",								cmdOLC,				0,															TRUST_STAFF			},

	{	"pack",								cmdPack,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"pets",								cmdPets,			0,															TRUST_PLAYER		},
	{	"prefix",							cmdPrefix,			0,															TRUST_PLAYER		},
	{	"prepare",							cmdPrepare,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"prompt",							cmdPrompt,			0,															TRUST_PLAYER		},

	{	"purge",							cmdPurge,			0,															TRUST_STAFF			},

	{	"qclear",							cmdQClear,			0,															TRUST_PLAYER		},
	{	"qlist",							cmdQList,			0,															TRUST_PLAYER		},
	{	"quest",							cmdQuest,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"quit",								cmdQuit,			CMD_BALANCED,												TRUST_PLAYER		},

	{	"qfind",							cmdQuestFind,		0,															TRUST_STAFF			},
	{	"qedit",							cmdQedit,			0,															TRUST_STAFF			},

	{	"retreat",							cmdRetreat,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"relax",							cmdRelax,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"roll",								cmdRoll,			0,															TRUST_PLAYER		},

	{	"read",								cmdRead,			0,															TRUST_PLAYER		},
	{	"recipes",							cmdRecipes,			0,															TRUST_PLAYER		},
	{	"remove",							cmdRemove,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"remember",							cmdRemember,		0,															TRUST_PLAYER		},
	{	"replace",							cmdReplace,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"reply",							cmdReply,			0,															TRUST_PLAYER		},

	{	"redit",							cmdRedit,			0,															TRUST_STAFF			},
	{	"restore",							cmdRestore,			0,															TRUST_STAFF			},

	{	"say",								cmdSay,				0,															TRUST_PLAYER		},
	{	"sayto",							cmdSayTo,			0,															TRUST_PLAYER		},
	{	"score",							cmdScore,			0,															TRUST_PLAYER		},
	{	"scavenge",							cmdScavenge,		CMD_BALANCED,												TRUST_PLAYER		},
	{	"search",							cmdSearch,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"sell",								cmdSell,			CMD_EXACT | CMD_BALANCED,									TRUST_PLAYER		},
	{	"shout",							cmdShout,			0,															TRUST_PLAYER		},
	{	"sit",								cmdSit,				0,															TRUST_PLAYER		},
	{	"skills",							cmdSkills,			0,															TRUST_PLAYER		},
	{	"slots",							cmdSlots,			0,															TRUST_PLAYER		},
	{	"sort",								cmdSort,			0,															TRUST_PLAYER		},
	{	"spells",							cmdSpells,			0,															TRUST_PLAYER		},
	{	"stats",							cmdStats,			0,															TRUST_PLAYER		},
	{	"stand",							cmdStand,			0,															TRUST_PLAYER		},
	{	"steal",							cmdSteal,			CMD_BALANCED | CMD_HIDDEN,									TRUST_PLAYER		},
	{	"suffix",							cmdSuffix,			0,															TRUST_PLAYER		},

	{	"save",								cmdSave,			CMD_EXACT,													TRUST_ADMIN			},
	{	"shfind",							cmdShopFind,		0,															TRUST_STAFF			},
	{	"shutdown",							cmdShutdown,		CMD_EXACT,													TRUST_ADMIN			},
	{	"slay",								cmdSlay,			0,															TRUST_STAFF			},
	{	"snoop",							cmdSnoop,			0,															TRUST_STAFF			},
	{	"surname",							cmdSurname,			0,															TRUST_GUILDMASTER	},

	{	"target",							cmdTarget,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"tattoo",							cmdTattoo,			CMD_BALANCED,												TRUST_PLAYER		},

	{	"take",								cmdTake,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"techniques",						cmdTechniques,		0,															TRUST_PLAYER		},
	{	"tell",								cmdTell,			0,															TRUST_PLAYER		},
	{	"time",								cmdTime,			0,															TRUST_PLAYER		},
	{	"titles",							cmdTitles,			0,															TRUST_PLAYER		},
	{	"tradeskills",						cmdTradeskills,		0,															TRUST_PLAYER		},
	{	"train",							cmdTrain,			CMD_EXACT | CMD_BALANCED,									TRUST_PLAYER		},
	{	"typo",								cmdTypo,			CMD_EXACT,													TRUST_PLAYER		},

	{	"transfer",							cmdTransfer,		0,															TRUST_STAFF			},

	{	"unhide",							cmdUnhide,			CMD_BALANCED | CMD_HIDDEN,									TRUST_PLAYER		},
	{	"unpack",							cmdUnpack,			CMD_BALANCED,												TRUST_PLAYER		},

	{	"update",							cmdUpdate,			0,															TRUST_STAFF			},
	{	"users",							cmdUsers,			0,															TRUST_ADMIN			},

	{	"vault",							cmdVault,			CMD_BALANCED,												TRUST_PLAYER		},

	{	"moveto",							cmdMoveto,			CMD_BALANCED | CMD_EXACT,									TRUST_PLAYER		},
	{	"where",							cmdWhere,			0,															TRUST_PLAYER		},
	{	"who",								cmdWho,				0,															TRUST_PLAYER		},
	{	"wear",								cmdWear,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"wield",							cmdWield,			CMD_BALANCED,												TRUST_PLAYER		},
	{	"withdraw",							cmdWithdraw,		CMD_BALANCED,												TRUST_PLAYER		},

	{	"zfind",							cmdZoneFind,		0,															TRUST_STAFF			},
	{	"zedit",							cmdZedit,			0,															TRUST_STAFF			},

	{	NULL,								NULL,				0,															TRUST_ADMIN			}
};

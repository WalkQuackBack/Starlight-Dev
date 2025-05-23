#include "PhiveMaterialData.h"

std::vector<const char*> PhiveMaterialData::mMaterialNames =
{
	"Undefined",
	"Soil",
	"Grass",
	"Sand",
	"HeavySand",
	"Snow",
	"HeavySnow",
	"Stone",
	"StoneSlip",
	"StoneNoSlip",
	"SlipBoard",
	"Cart",
	"Metal",
	"MetalSlip",
	"MetalNoSlip",
	"WireNet",
	"Wood",
	"Ice",
	"Cloth",
	"Glass",
	"Bone",
	"Rope",
	"Character",
	"Ragdoll",
	"Surfing",
	"GuardianFoot",
	"LaunchPad",
	"Conveyer",
	"Rail",
	"Grudge",
	"Meat",
	"Vegetable",
	"Bomb",
	"MagicBall",
	"Barrier",
	"AirWall",
	"GrudgeSnow",
	"Tar",
	"Water",
	"HotWater",
	"IceWater",
	"Lava",
	"Bog",
	"ContaminatedWater",
	"DungeonCeil",
	"Gas",
	"InvalidateRestartPos",
	"HorseSpeedLimit",
	"ForbidDynamicCuttingAreaForHugeCharacter",
	"ForbidHorseReturnToSafePosByClimate",
	"Dragon",
	"ReferenceSurfing",
	"WaterSlip",
	"HorseDeleteImmediatelyOnDeath"
};

std::vector<const char*> PhiveMaterialData::mMaterialFlagNames =
{
	"DisableSurfaceVelocity",
	"DisableRespawn",
	"NoStick",
	"NoClimb",
	"Ladder",
	"LadderUp",
	"LadderSide",
	"Slip",
	"NoSlipRainOnClimb",
	"NoDashUpAndNoClimb",
	"Abyss",
	"TopBroadLeafTree",
	"TopConiferousTree",
	"Attach",
	"NarrowPlace",
	"StopUntilStickNeutral",
	"EdgeOfTheWorld",
	"Wet",
	"EnableWallJump",
	"RoofWall_InCamera",
	"RoofWall_OutCamera_NotStat",
	"HyurleCastle",
	"RoofWall_OutCamera",
	"Area_Roof_OutCamera",
	"Remains",
	"Roof_InCamera",
	"FlowStraight",
	"FlowLeft",
	"FlowRight",
	"NoImpulseUpperMove",
	"NoPreventFall",
	"DontIgnoreInertiaForce",
	"HeadShot",
	"DontGenerateNavMesh",
	"Unridable",
	"SightDown",
	"DisableImpulse",
	"Miasma",
	"MiasmaLv1",
	"MiasmaLv3",
	"IgnoredByNavMesh",
	"CeilingClipperForceAccept",
	"StopCeilingClipper",
	"Spike_Y_Minus",
	"ForbidAutoPlacementAndEnemyEntry",
	"ForbidAutoPlacementAll",
	"Dragon",
	"BakeInstanceID",
	"NoEntrySage",
	"MiasmaCancel",
	"Sunlight",
	"Confusion",
	"SkyLomeiWall",
	"Fruit",
	"ShrineEntrance",
	"NavMeshNPCOnly",
	"NavMeshGiantNoEntry",
	"CameraAtBlock",
	"MiasmaEntity",
	"IgnoreAttach",
	"IgnoreBondConnection",
	"SwitchHit",
	"TrolleyRotationFloor",
	"InvalidateRestartPosForHorse"
};

std::vector<uint64_t> PhiveMaterialData::mMaterialFlagMasks = 
{
	33554432,
	16777216,
	32768,
	1,
	4,
	2,
	16,
	8,
	32,
	65,
	128,
	512,
	1024,
	0,
	256,
	134217728,
	8388608,
	4194304,
	2097152,
	458752,
	917504,
	1048576,
	393216,
	262144,
	0,
	327680,
	0,
	0,
	0,
	0,
	16384,
	268435456,
	536870912,
	1073741824,
	2147483648,
	4294967296,
	67108864,
	8589934592,
	34359738368,
	576460752303423488,
	8796093022208,
	17592186044416,
	35184372088832,
	70368744177664,
	290202350256128,
	285804303745024,
	140737555464192,
	562949953421312,
	1125899906842624,
	2251799813685248,
	4503599627370496,
	9007199254740992,
	18014398509482049,
	36028797018963968,
	1224979098644774912,
	144115188075855872,
	288230376151711744,
	1152921504606846976,
	2305843009213693952,
	4611686018427387904,
	9223372036854775808,
	18014398509482049,
	4096,
	8192
};

std::vector<const char*> PhiveMaterialData::mMaterialFilterNames =
{
	"HitAll",
	"FilterAnimal",
	"FilterNPC",
	"FilterAttackHitEnemy",
	"FilterArrow",
	"FilterArrowToEnsureSensorHit",
	"FilterBlast",
	"FilterUltraHand",
	"FilterIK",
	"FilterMovingTrolley",
	"FilterAirWall",
	"FilterUnspecified",
	"HitLiving",
	"HitLivingAndIK",
	"HitOnlyAnimal",
	"HitOnlyIK",
	"NoHit",
	"AirWallForClimb",
	"FilterChemical",
	"FilterLight",
	"FloatingWater",
	"WaterCancel",
	"HitOnlyDetectMarking",
	"FilterWaterfall",
	"PlayerClimb",
	"FilterCeilingClipper",
	"FilterBeam",
	"Smoke",
	"Default",
	"HitOnlySage",
	"FilterSage",
	"FilterShutter",
	"DrillingArea",
	"AirWallOnly",
	"AirWallSageEnable",
	"AirWallSageBlockable",
	"FilterSageAndSoulSage",
	"LiftRock",
	"FilterAtTgOcculsionCheck",
	"FilterDiveAttacking",
	"FilterDestructible",
	"HitOnlyCaveMaster",
	"LumberedTree",
	"HitOnlyIce",
	"FilterTrackingKinematicBody",
	"HitOnlyRotatingTurntable",
	"LightOnly",
	"IvyBurn",
	"HitOnlyHugeCharacter",
	"HitOnlyEnemyVehicle",
	"HitOnlyNpc",
	"HitOnlyEnemy",
	"HitOnlyCeilingClipper",
	"HitOnlyEnemyVehicleAndAirWall",
	"BossRitoIceBoard",
	"HitOnlyPlayer",
	"FilterEnemy",
	"FilterFloatingWater",
	"FilterNpcAndFloatingWater",
	"PlayerAttackGraveHit",
	"FootStep"
	"BalloonAirWall",
	"HitOnlyBeam",
	"FilterClimbingPlayer",
	"HitOnlyPlayerAndBlast",
	"FilterBondConnection",
	"AirWallForGerudoDungeon",
};

std::vector<uint64_t> PhiveMaterialData::mMaterialFilterMasks =
{
	0xffffffff,
	0xfffffffd,
	0xfffffff7,
	0xffffffdf,
	0xf7bf9f3f,
	0xf7ffff3f,
	0xffff9f7f,
	0xffffffff,
	0xfffffdff,
	0xfffffbff,
	0xffffefff,
	0xfffffffe,
	0x3300001f,
	0x3300021f,
	0x00000002,
	0x00000200,
	0x00000000,
	0x00000010,
	0xffbf9f7f,
	0xffffdfff,
	0xffff7fff,
	0x00020000,
	0x00040000,
	0xfff7ffff,
	0xffffe7ff,
	0xffefffff,
	0xffdfffff,
	0xf783301f,
	0xffffffff,
	0x01000000,
	0xfeffffff,
	0xffbf9f3f,
	0x0c000000,
	0x00001000,
	0x21000004,
	0x01000000,
	0xfcffffff,
	0xfffffffd,
	0xf7ffffff,
	0xefffffff,
	0xfbffffff,
	0x20000000,
	0xffffff7f,
	0x00010000,
	0xbfffffff,
	0x80000000,
	0x00002000,
	0x00000001,
	0x1000000f,
	0x00000100,
	0x00000008,
	0x00000004,
	0x00100000,
	0x00001100,
	0xffffffbf,
	0x10000011,
	0xfffffffb,
	0xffff7fff,
	0xffff7ff7,
	0xbffffffb,
	0xffffefff,
	0x3310001f,
	0x00200000,
	0xffffffef,
	0x10000091,
	0x7fffffff,
	0x03000000
};

std::vector<bool> PhiveMaterialData::mMaterialFilterIsBase =
{
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	true,
	true,
	true,
	true,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	true,
	false,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	false,
	false,
	false,
	false,
	false,
	true,
	true,
	false,
	true,
	false,
	true
};
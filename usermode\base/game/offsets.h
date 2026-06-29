#include <d3dx9math.h>
namespace offsets
{
    //Class Addrs
    uintptr_t m_World, //48 8B 05 ? ? ? ? 33 D2 48 8B 40 08 8A CA 48 85 C0 74 16 48 8B
        m_ReplayInterFace, //48 8D 0D ?? ?? ?? ?? 48 ?? ?? E8 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? 8A D8 E8 ?? ?? ?? ?? 84 DB 75 13 48 8D 0D ?? ?? ?? ?? 48 8B D7 E8 ?? ?? ?? ?? 84 C0 74 BC 8B 8F
        m_ViewPort, //48 8B 15 ? ? ? ? 48 8D 2D ? ? ? ? 48 8B CD
        m_BlipList, //4C 8D 05 ? ? ? ? 0F B7 C1
        m_CamGameplayDirector, //4C 8B 35 ? ? ? ? 33 FF 32 DB
        m_HandleBullet,
        m_LocalPlayer;


    //Function Addrs
    uintptr_t m_SilentAim,
        m_InfiniteAmmo0,
        m_InfiniteAmmo1,
        m_MagicBulletsPatch,
        m_ArmsKinematics,
        m_LegsKinematics,
        m_InfiniteCombatRoll,
        m_GiveWeapon;

    //Offsets
    uintptr_t m_LastVehicle,
        m_Handling,
        m_VehicleEngineHealth,
        m_PlayerInfo,
        m_FragInst,
        m_Armor,
        m_Recoil,
        m_PedFlag,
        m_Spread,
        m_WeaponManager,
        m_WeaponInfo,
        m_VehicleDoorsLockState,
        m_EntityType,
        m_Speed,
        m_MaxHealth,
        m_VehicleDriver,
        m_VehicleGravity,
        m_CObject,
        m_NoRagDoll,
        m_CWeapon,
        m_PlayerId,
        m_CitizenNamesModBase,
        m_NetIdToNamesEntry,
        m_SeatBealt,
        m_AimCPedPatternResult,
        m_FrameFlag,
        m_VehicleState,
        m_FreecanPatch;

    //Infos
    int CurrentBuild;
    std::string ServerIp;

}

#pragma region Enums
enum BoneMasks : int { SKEL_ROOT = 0x0, SKEL_Pelvis = 0x2e28, SKEL_L_Thigh = 0xe39f, SKEL_L_Calf = 0xf9bb, SKEL_L_Foot = 0x3779, SKEL_L_Toe0 = 0x83c, IK_L_Foot = 0xfedd, PH_L_Foot = 0xe175, MH_L_Knee = 0xb3fe, SKEL_R_Thigh = 0xca72, SKEL_R_Calf = 0x9000, SKEL_R_Foot = 0xcc4d, SKEL_R_Toe0 = 0x512d, IK_R_Foot = 0x8aae, PH_R_Foot = 0x60e6, MH_R_Knee = 0x3fcf, RB_L_ThighRoll = 0x5c57, RB_R_ThighRoll = 0x192a, SKEL_Spine_Root = 0xe0fd, SKEL_Spine0 = 0x5c01, SKEL_Spine1 = 0x60f0, SKEL_Spine2 = 0x60f1, SKEL_Spine3 = 0x60f2, SKEL_L_Clavicle = 0xfcd9, SKEL_L_UpperArm = 0xb1c5, SKEL_L_Forearm = 0xeeeb, SKEL_L_Hand = 0x49d9, SKEL_L_Finger00 = 0x67f2, SKEL_L_Finger01 = 0xff9, SKEL_L_Finger02 = 0xffa, SKEL_L_Finger10 = 0x67f3, SKEL_L_Finger11 = 0x1049, SKEL_L_Finger12 = 0x104a, SKEL_L_Finger20 = 0x67f4, SKEL_L_Finger21 = 0x1059, SKEL_L_Finger22 = 0x105a, SKEL_L_Finger30 = 0x67f5, SKEL_L_Finger31 = 0x1029, SKEL_L_Finger32 = 0x102a, SKEL_L_Finger40 = 0x67f6, SKEL_L_Finger41 = 0x1039, SKEL_L_Finger42 = 0x103a, PH_L_Hand = 0xeb95, IK_L_Hand = 0x8cbd, RB_L_ForeArmRoll = 0xee4f, RB_L_ArmRoll = 0x1470, MH_L_Elbow = 0x58b7, SKEL_R_Clavicle = 0x29d2, SKEL_R_UpperArm = 0x9d4d, SKEL_R_Forearm = 0x6e5c, SKEL_R_Hand = 0xdead, SKEL_R_Finger00 = 0xe5f2, SKEL_R_Finger01 = 0xfa10, SKEL_R_Finger02 = 0xfa11, SKEL_R_Finger10 = 0xe5f3, SKEL_R_Finger11 = 0xfa60, SKEL_R_Finger12 = 0xfa61, SKEL_R_Finger20 = 0xe5f4, SKEL_R_Finger21 = 0xfa70, SKEL_R_Finger22 = 0xfa71, SKEL_R_Finger30 = 0xe5f5, SKEL_R_Finger31 = 0xfa40, SKEL_R_Finger32 = 0xfa41, SKEL_R_Finger40 = 0xe5f6, SKEL_R_Finger41 = 0xfa50, SKEL_R_Finger42 = 0xfa51, PH_R_Hand = 0x6f06, IK_R_Hand = 0x188e, RB_R_ForeArmRoll = 0xab22, RB_R_ArmRoll = 0x90ff, MH_R_Elbow = 0xbb0, SKEL_Neck_1 = 0x9995, SKEL_Head = 0x796e, IK_Head = 0x322c, FACIAL_facialRoot = 0xfe2c, FB_L_Brow_Out_000 = 0xe3db, FB_L_Lid_Upper_000 = 0xb2b6, FB_L_Eye_000 = 0x62ac, FB_L_CheekBone_000 = 0x542e, FB_L_Lip_Corner_000 = 0x74ac, FB_R_Lid_Upper_000 = 0xaa10, FB_R_Eye_000 = 0x6b52, FB_R_CheekBone_000 = 0x4b88, FB_R_Brow_Out_000 = 0x54c, FB_R_Lip_Corner_000 = 0x2ba6, FB_Brow_Centre_000 = 0x9149, FB_UpperLipRoot_000 = 0x4ed2, FB_UpperLip_000 = 0xf18f, FB_L_Lip_Top_000 = 0x4f37, FB_R_Lip_Top_000 = 0x4537, FB_Jaw_000 = 0xb4a0, FB_LowerLipRoot_000 = 0x4324, FB_LowerLip_000 = 0x508f, FB_L_Lip_Bot_000 = 0xb93b, FB_R_Lip_Bot_000 = 0xc33b, FB_Tongue_000 = 0xb987, RB_Neck_1 = 0x8b93, IK_Root = 0xdd1c };
enum ePedConfigFlag { NoCriticalHits = 2, DrownsInWater = 3, DisableReticuleFixedLockon = 4, UpperBodyDamageAnimsOnly = 7, NeverLeavesGroup = 13, BlockNonTemporaryEvents = 17, CanPunch = 18, IgnoreSeenMelee = 24, GetOutUndriveableVehicle = 29, CanFlyThruWindscreen = 32, DiesWhenRagdoll = 33, HasHelmet = 34, PutOnMotorcycleHelmet = 35, DontTakeOffHelmet = 36, DisableEvasiveDives = 39, DontInfluenceWantedLevel = 42, DisablePlayerLockon = 43, DisableLockonToRandomPeds = 44, AllowLockonToFriendlyPlayers = 45, BeingDeleted = 47, BlockWeaponSwitching = 48, NoCollision = 52, IsShooting = 58, WasShooting = 59, IsOnGround = 60, WasOnGround = 61, InVehicle = 62, OnMount = 63, AttachedToVehicle = 64, IsSwimming = 65, WasSwimming = 66, IsSkiing = 67, IsSitting = 68, KilledByStealth = 69, KilledByTakedown = 70, Knockedout = 71, IsSniperScopeActive = 72, SuperDead = 73, UsingCoverPoint = 75, IsInTheAir = 76, IsAimingGun = 78, ForcePedLoadCover = 93, VaultFromCover = 97, IsDrunk = 100, ForcedAim = 101, IsNotRagdollAndNotPlayingAnim = 104, ForceReload = 105, DontActivateRagdollFromVehicleImpact = 106, DontActivateRagdollFromBulletImpact = 107, DontActivateRagdollFromExplosions = 108, DontActivateRagdollFromFire = 109, DontActivateRagdollFromElectrocution = 110, KeepWeaponHolsteredUnlessFired = 113, GetOutBurningVehicle = 116, BumpedByPlayer = 117, RunFromFiresAndExplosions = 118, TreatAsPlayerDuringTargeting = 119, IsHandCuffed = 120, IsAnkleCuffed = 121, DisableMelee = 122, DisableUnarmedDrivebys = 123, JustGetsPulledOutWhenElectrocuted = 124, NmMessage466 = 125, WillNotHotwireLawEnforcementVehicle = 126, WillCommandeerRatherThanJack = 127, CanBeAgitated = 128, ForcePedToFaceLeftInCover = 129, ForcePedToFaceRightInCover = 130, BlockPedFromTurningInCover = 131, KeepRelationshipGroupAfterCleanUp = 132, ForcePedToBeDragged = 133, PreventPedFromReactingToBeingJacked = 134, IsScuba = 135, WillArrestRatherThanJack = 136, RemoveDeadExtraFarAway = 137, RidingTrain = 138, ArrestResult = 139, CanAttackFriendly = 140, WillJackAnyPlayer = 141, WillJackWantedPlayersRatherThanStealCar = 144, ShootingAnimFlag = 145, DisableLadderClimbing = 146, StairsDetected = 147, SlopeDetected = 148, CowerInsteadOfFlee = 150, CanActivateRagdollWhenVehicleUpsideDown = 151, AlwaysRespondToCriesForHelp = 152, DisableBloodPoolCreation = 153, ShouldFixIfNoCollision = 154, CanPerformArrest = 155, CanPerformUncuff = 156, CanBeArrested = 157, PlayerPreferFrontSeatMP = 159, IsInjured = 166, DontEnterVehiclesInPlayersGroup = 167, PreventAllMeleeTaunts = 169, IsInjured2 = 170, AlwaysSeeApproachingVehicles = 171, CanDiveAwayFromApproachingVehicles = 172, AllowPlayerToInterruptVehicleEntryExit = 173, OnlyAttackLawIfPlayerIsWanted = 174, PedsJackingMeDontGetIn = 177, PedIgnoresAnimInterruptEvents = 179, IsInCustody = 180, ForceStandardBumpReactionThresholds = 181, LawWillOnlyAttackIfPlayerIsWanted = 182, IsAgitated = 183, PreventAutoShuffleToDriversSeat = 184, UseKinematicModeWhenStationary = 185, EnableWeaponBlocking = 186, HasHurtStarted = 187, DisableHurt = 188, PlayerIsWeird = 189, DoNothingWhenOnFootByDefault = 193, UsingScenario = 194, VisibleOnScreen = 195, DontActivateRagdollOnVehicleCollisionWhenDead = 199, HasBeenInArmedCombat = 200, AvoidanceIgnoreAll = 202, AvoidanceIgnoredByAll = 203, AvoidanceIgnoreGroup1 = 204, AvoidanceMemberOfGroup1 = 205, ForcedToUseSpecificGroupSeatIndex = 206, DisableExplosionReactions = 208, DodgedPlayer = 209, WaitingForPlayerControlInterrupt = 210, ForcedToStayInCover = 211, GeneratesSoundEvents = 212, ListensToSoundEvents = 213, AllowToBeTargetedInAVehicle = 214, WaitForDirectEntryPointToBeFreeWhenExiting = 215, OnlyRequireOnePressToExitVehicle = 216, ForceExitToSkyDive = 217, DontEnterLeadersVehicle = 220, DisableExitToSkyDive = 221, Shrink = 223, MeleeCombat = 224, DisablePotentialToBeWalkedIntoResponse = 225, DisablePedAvoidance = 226, ForceRagdollUponDeath = 227, DisablePanicInVehicle = 229, AllowedToDetachTrailer = 230, IsHoldingProp = 236, BlocksPathingWhenDead = 237, ForceSkinCharacterCloth = 240, DisableStoppingVehicleEngine = 241, PhoneDisableTextingAnimations = 242, PhoneDisableTalkingAnimations = 243, PhoneDisableCameraAnimations = 244, DisableBlindFiringInShotReactions = 245, AllowNearbyCoverUsage = 246, CanPlayInCarIdles = 248, CanAttackNonWantedPlayerAsLaw = 249, WillTakeDamageWhenVehicleCrashes = 250, AICanDrivePlayerAsRearPassenger = 251, PlayerCanJackFriendlyPlayers = 252, IsOnStairs = 253, AIDriverAllowFriendlyPassengerSeatEntry = 255, AllowMissionPedToUseInjuredMovement = 257, PreventUsingLowerPrioritySeats = 261, DisableClosingVehicleDoor = 264, TeleportToLeaderVehicle = 268, AvoidanceIgnoreWeirdPedBuffer = 269, OnStairSlope = 270, DontBlipCop = 272, ClimbedShiftedFence = 273, KillWhenTrapped = 275, EdgeDetected = 276, AvoidTearGas = 279, NoWrithe = 281, OnlyUseForcedSeatWhenEnteringHeliInGroup = 282, DisableWeirdPedEvents = 285, ShouldChargeNow = 286, RagdollingOnBoat = 287, HasBrandishedWeapon = 288, FreezePosition = 292, DisableShockingEvents = 294, NeverReactToPedOnRoof = 296, DisableShockingDrivingOnPavementEvents = 299, DisablePedConstraints = 301, ForceInitialPeekInCover = 302, DisableJumpingFromVehiclesAfterLeader = 305, IsInCluster = 310, ShoutToGroupOnPlayerMelee = 311, IgnoredByAutoOpenDoors = 312, NoPedMelee = 314, CheckLoSForSoundEvents = 315, CanSayFollowedByPlayerAudio = 317, ActivateRagdollFromMinorPlayerContact = 318, ForcePoseCharacterCloth = 320, HasClothCollisionBounds = 321, HasHighHeels = 322, DontBehaveLikeLaw = 324, DisablePoliceInvestigatingBody = 326, DisableWritheShootFromGround = 327, LowerPriorityOfWarpSeats = 328, DisableTalkTo = 329, DontBlip = 330, IsSwitchingWeapon = 331, IgnoreLegIkRestrictions = 332, AllowTaskDoNothingTimeslicing = 339, NotAllowedToJackAnyPlayers = 342, AlwaysLeaveTrainUponArrival = 345, OnlyWritheFromWeaponDamage = 347, UseSloMoBloodVfx = 348, EquipJetpack = 349, PreventDraggedOutOfCarThreatResponse = 350, ForceDeepSurfaceCheck = 356, DisableDeepSurfaceAnims = 357, DontBlipNotSynced = 358, IsDuckingInVehicle = 359, PreventAutoShuffleToTurretSeat = 360, DisableEventInteriorStatusCheck = 361, HasReserveParachute = 362, UseReserveParachute = 363, TreatDislikeAsHateWhenInCombat = 364, OnlyUpdateTargetWantedIfSeen = 365, AllowAutoShuffleToDriversSeat = 366, PreventReactingToSilencedCloneBullets = 372, DisableInjuredCryForHelpEvents = 373, NeverLeaveTrain = 374, DontDropJetpackOnDeath = 375, DisableAutoEquipHelmetsInBikes = 380, IsClimbingLadder = 388, HasBareFeet = 389, GoOnWithoutVehicleIfItIsUnableToGetBackToRoad = 391, BlockDroppingHealthSnacksOnDeath = 392, ForceThreatResponseToNonFriendToFriendMeleeActions = 394, DontRespondToRandomPedsDamage = 395, AllowContinuousThreatResponseWantedLevelUpdates = 396, KeepTargetLossResponseOnCleanup = 397, PlayersDontDragMeOutOfCar = 398, BroadcastRepondedToThreatWhenGoingToPointShooting = 399, IgnorePedTypeForIsFriendlyWith = 400, TreatNonFriendlyAsHateWhenInCombat = 401, DontLeaveVehicleIfLeaderNotInVehicle = 402, AllowMeleeReactionIfMeleeProofIsOn = 404, UseNormalExplosionDamageWhenBlownUpInVehicle = 407, DisableHomingMissileLockForVehiclePedInside = 408, DisableTakeOffScubaGear = 409, Alpha = 410, LawPedsCanFleeFromNonWantedPlayer = 411, ForceBlipSecurityPedsIfPlayerIsWanted = 412, IsHolsteringWeapon = 413, UseGoToPointForScenarioNavigation = 414, DontClearLocalPassengersWantedLevel = 415, BlockAutoSwapOnWeaponPickups = 416, ThisPedIsATargetPriorityForAI = 417, IsSwitchingHelmetVisor = 418, ForceHelmetVisorSwitch = 419, FlamingFootprints = 421, DisableVehicleCombat = 422, DisablePropKnockOff = 423, FallsLikeAircraft = 424, UseLockpickVehicleEntryAnimations = 426, IgnoreInteriorCheckForSprinting = 427, SwatHeliSpawnWithinLastSpottedLocation = 428, DisableStartingVehicleEngine = 429, IgnoreBeingOnFire = 430, DisableTurretOrRearSeatPreference = 431, DisableWantedHelicopterSpawning = 432, UseTargetPerceptionForCreatingAimedAtEvents = 433, DisableHomingMissileLockon = 434, ForceIgnoreMaxMeleeActiveSupportCombatants = 435, StayInDefensiveAreaWhenInVehicle = 436, DontShoutTargetPosition = 437, DisableHelmetArmor = 438, PreventVehExitDueToInvalidWeapon = 441, IgnoreNetSessionFriendlyFireCheckForAllowDamage = 442, DontLeaveCombatIfTargetPlayerIsAttackedByPolice = 443, CheckLockedBeforeWarp = 444, DontShuffleInVehicleToMakeRoom = 445, GiveWeaponOnGetup = 446, DontHitVehicleWithProjectiles = 447, DisableForcedEntryForOpenVehiclesFromTryLockedDoor = 448, FiresDummyRockets = 449, IsArresting = 450, IsDecoyPed = 451, HasEstablishedDecoy = 452, BlockDispatchedHelicoptersFromLanding = 453, DontCryForHelpOnStun = 454, CanBeIncapacitated = 456, MutableForcedAim = 457, DontChangeTargetFromMelee = 458 };
#pragma endregion

class CPed;
class CVehicle;
class CPlayerInfo;
class CWeaponManager;

#pragma region Infos
class CPlayerInfo {
public:
	int PlayerID() {
		if (!this) { return 0; }
		return client.read<int>(reinterpret_cast<uintptr_t>(this) + offsets::m_PlayerId);
	}

	void SetInfStamina(bool Toggle) {
		if (!this) { return; }
		//Mem.Write<float>(reinterpret_cast<uintptr_t>(this) + 0xCF4, Toggle ? FLT_MAX : 100);
	}
};


#pragma endregion

#pragma region List
class CVehicleList {
public:
	CVehicle* Vehicle(int Idx)
	{
		if (!this) { return 0; }
		return (CVehicle*)client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + (Idx * 0x10U));
	}

};

class CPedList {
public:
	CPed* Ped(int Idx) {
		if (!this) { return 0; }
		return (CPed*)client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + (Idx * 0x10U));
	}
};
#pragma endregion

#pragma region InterFaces
class CPedInterFace {
public:
	int MaxPed() { return client.read<int>(reinterpret_cast<uintptr_t>(this) + 0x108); }
	int PedCount() { return client.read<int>(reinterpret_cast<uintptr_t>(this) + 0x110); }
	CPedList* PedList() { return (CPedList*)client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x100); }
};

class CVehInterFace {
public:
	int MaxVehicles() { return client.read<int>(reinterpret_cast<uintptr_t>(this) + 0x188); }
	int VehicleCount() { return client.read<int>(reinterpret_cast<uintptr_t>(this) + 0x190); }
	CVehicleList* VehicleList() { return (CVehicleList*)client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x180); }
};

class CReplayInterFace {
public:

	CPedInterFace* InterfacePed() {
		if (!this) { return 0; }
		return (CPedInterFace*)client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x18);
	}

	CVehInterFace* InterfaceVeh() {
		if (!this) { return 0; }
		return (CVehInterFace*)client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x10);
	}
};

#pragma endregion

class CPed {
public:

	D3DXVECTOR3 m_vecPosition;

	float GetMaxHealth() {
		if (!this) { return 0; }
		return client.read<float>(reinterpret_cast<uintptr_t>(this) + offsets::m_MaxHealth);
	}

	float GetHealth() {
		if (!this) { return 0; }
		return client.read<float>(reinterpret_cast<uintptr_t>(this) + 0x280);
	}

	void SetHealth(float Health) {
		if (!this) { return; }
		//Mem.Write<float>(reinterpret_cast<uintptr_t>(this) + 0x280, Health);
	}

	float GetArmor() {
		if (!this) { return 0; }
		return client.read<float>(reinterpret_cast<uintptr_t>(this) + offsets::m_Armor);
	}

	void SetArmor(float Armor) {
		if (!this) { return; }
		//Mem.Write<float>(reinterpret_cast<uintptr_t>(this) + offsets::m_Armor, Armor);
	}

	uint64_t GetModelInfo()
	{
		if (!this)
			return 0;

		return client.read<uint64_t>(reinterpret_cast<uintptr_t>(this) + 0x20);
	}

	float GetSpeed() {
		if (!this) { return 0; }
		CPlayerInfo* PlayerInfo = this->GetPlayerInfo();
		return client.read<float>(reinterpret_cast<uintptr_t>(PlayerInfo) + offsets::m_Speed);
	}

	void SetSpeed(float Speed) {
		if (!this) { return; }
		CPlayerInfo* PlayerInfo = this->GetPlayerInfo();
		//Mem.Write<float>(reinterpret_cast<uintptr_t>(PlayerInfo) + offsets::m_Speed, Speed);
	}

	D3DXVECTOR3 GetPos() {
		if (!this) { return D3DXVECTOR3(0, 0, 0); }
		return client.read<D3DXVECTOR3>(reinterpret_cast<uintptr_t>(this) + 0x90);
	}

	void SetPos(D3DXVECTOR3 Pos) {
		if (!this) { return; }
		bool InsideVehicle = InVehicle();
		uintptr_t LastVeh = reinterpret_cast<uintptr_t>(GetLastVehicle());

		if (LastVeh && InsideVehicle) {
			uintptr_t Navigation = client.read<uintptr_t>(LastVeh + 0x30);
			//Mem.Write<D3DXVECTOR3>(Navigation + 0x30, D3DXVECTOR3(0, 0, 0));
			//Mem.Write<D3DXVECTOR3>(LastVeh + 0x90, Pos);
		}
		else if (!InsideVehicle) {
			uintptr_t Navigation = this->GetNavigation();
			//Mem.Write<D3DXVECTOR3>(Navigation + 0x30, D3DXVECTOR3{ 0, 0, 0 });
			//Mem.Write<D3DXVECTOR3>(reinterpret_cast<uintptr_t>(this) + 0x90, Pos);
		}
	}

	void SeatBealt(bool toggle) {
		if (!this) { return; }
		bool InsideVehicle = InVehicle();

		if (InsideVehicle) {
			if (toggle) {
			//	uintptr_t Path = Mem.FindSignature({ 0x83, 0xa1, 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x83, 0xe2 });
				//Mem.WriteBytes(Path, { 0x90,0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, });
				//Mem.Write<BYTE>((uintptr_t)this + offsets::m_SeatBealt, 0xC9);
			}
			else {
				//Mem.Write<BYTE>((uintptr_t)this + offsets::m_SeatBealt, 0xC8);
			}
		}

	}

	CVehicle* GetLastVehicle() {
		if (!this) return 0;
		return (CVehicle*)client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + offsets::m_LastVehicle);
	}

	CPlayerInfo* GetPlayerInfo() {
		if (!this) return 0;
		return (CPlayerInfo*)client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + offsets::m_PlayerInfo);
	}

	CWeaponManager* GetWeaponManager() {
		if (!this) return 0;
		return (CWeaponManager*)client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + offsets::m_WeaponManager);
	}

	uint32_t GetPedType() {
		if (!this) { return 0; }
		return client.read<uint32_t>(reinterpret_cast<uintptr_t>(this) + offsets::m_EntityType) << 11 >> 25;
	}

	int GetID() {
		if (!this) { return 0; }
		CPlayerInfo* PlayerInfo = (CPlayerInfo*)GetPlayerInfo();
		int Id = PlayerInfo->PlayerID();
		return Id;
	}

	bool HasFlag(ePedConfigFlag Flag)
	{
		if (!this) { return false; }

		auto v1 = (int)Flag;
		if (!this || v1 > 0x1CA) return false;

		auto v2 = 1 << (v1 & 0x1F);
		auto v3 = v1 >> 5;
		auto v4 = reinterpret_cast<uintptr_t>(this) + 4 * v3 + offsets::m_PedFlag;
		auto v5 = client.read<long>(v4);

		return (v2 & v5) != 0;
	}

	void SetConfigFlag(ePedConfigFlag Flag, bool Value)
	{
		if (!this) { return; }

		auto v1 = (int)Flag;
		if (!this || v1 > 0x1CA) return;

		auto v2 = 1 << (v1 & 0x1F);
		auto v3 = v1 >> 5;
		auto v4 = (uintptr_t)(this) + 4 * v3 + offsets::m_PedFlag;
		auto v5 = client.read<long>(v4);

		if (Value != ((v2 & v5) != 0)) {
			auto v6 = v2 & (v5 ^ -(uint8_t)(Value ? 1 : 0));
			v5 ^= v6;
			//Mem.Write(v4, v5);
		}
	}

	bool IsVisible() {
		if (!this) return false;
		return HasFlag(ePedConfigFlag::VisibleOnScreen);
	}

	bool InVehicle() {
		if (!this) return false;
		return HasFlag(ePedConfigFlag::InVehicle);
	}

	void NoRagDoll(bool Toggle)
	{
		if (!this) return;
		//Mem.Write<BYTE>(reinterpret_cast<uintptr_t>(this) + offsets::m_NoRagDoll, Toggle ? 0x80 : 0x20);
	}


	D3DXVECTOR3 GetVelocity() {
		if (!this) { return D3DXVECTOR3(0, 0, 0); }
		return client.read<D3DXVECTOR3>(reinterpret_cast<uintptr_t>(this) + 0x320);
	}


	uintptr_t GetPlayerModel() {
		if (!this) { return 0; }
		uintptr_t CModelInfo = client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x20);
		return CModelInfo;
	}

	void FreezePed(bool Toggle) {
		if (!this) { return; }
		if (!InVehicle()) {
			uintptr_t CModelInfo = client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x20);
			//Mem.Write<float>(CModelInfo + 0x2C, Toggle ? 0.f : 1.f);
		}
		else {
			//Mem.Write<BYTE>(reinterpret_cast<uintptr_t>(GetLastVehicle()) + 0x2E, Toggle ? 1 : 2);
		}
	}

	void SetGodMode(bool Toggle) {
		if (!this) { return; }

		uintptr_t Addr = reinterpret_cast<uintptr_t>(this) + 0x188;
		DWORD flag = client.read<DWORD>(Addr);
		//Mem.Write<DWORD>(Addr, Toggle == true ? flag |= (1 << 9) : flag &= ~(1 << 9));
	}

	void SetInfStamina(bool Toggle) {
		if (!this) { return; }
		CPlayerInfo* PlayerInfo = (CPlayerInfo*)GetPlayerInfo();
		//Mem.Write<float>(reinterpret_cast<uintptr_t>(PlayerInfo) + 0xCF4, Toggle ? FLT_MAX : 100);
	}

	void BeastJump(bool Toggle) {
		if (!this) { return; }

		CPlayerInfo* PlayerInfo = (CPlayerInfo*)GetPlayerInfo();

		uintptr_t Addr = reinterpret_cast<uintptr_t>(PlayerInfo) + offsets::m_FrameFlag;
		DWORD flag = client.read<DWORD>(Addr);
		//Mem.Write<DWORD>(Addr, Toggle == true ? flag |= (1 << 28) : flag &= ~(1 << 28));
	}

	void SuperJump(bool Toggle) {
		if (!this) { return; }

		CPlayerInfo* PlayerInfo = (CPlayerInfo*)GetPlayerInfo();

		uintptr_t Addr = reinterpret_cast<uintptr_t>(PlayerInfo) + offsets::m_FrameFlag;
		DWORD flag = client.read<DWORD>(Addr);
		//Mem.Write<DWORD>(Addr, Toggle == true ? flag |= (1 << 14) : flag &= ~(1 << 14));
	}

	void ExplosiveAmmo(bool Toggle) {
		if (!this) { return; }

		CPlayerInfo* PlayerInfo = (CPlayerInfo*)GetPlayerInfo();

		uintptr_t Addr = reinterpret_cast<uintptr_t>(PlayerInfo) + offsets::m_FrameFlag;
		DWORD flag = client.read<DWORD>(Addr);
		//Mem.Write<DWORD>(Addr, Toggle == true ? flag |= (1 << 11) : flag &= ~(1 << 11));
	}

	void FireAmmo(bool Toggle) {
		if (!this) { return; }

		CPlayerInfo* PlayerInfo = (CPlayerInfo*)GetPlayerInfo();

		uintptr_t Addr = reinterpret_cast<uintptr_t>(PlayerInfo) + offsets::m_FrameFlag;
		DWORD flag = client.read<DWORD>(Addr);
		//Mem.Write<DWORD>(Addr, Toggle == true ? flag |= (1 << 12) : flag &= ~(1 << 12));
	}

	void ExplosiveFist(bool Toggle) {
		if (!this) { return; }

		CPlayerInfo* PlayerInfo = (CPlayerInfo*)GetPlayerInfo();

		uintptr_t Addr = reinterpret_cast<uintptr_t>(PlayerInfo) + offsets::m_FrameFlag;
		DWORD flag = client.read<DWORD>(Addr);
		//Mem.Write<DWORD>(Addr, Toggle == true ? flag |= (1 << 13) : flag &= ~(1 << 13));
	}

	void SuperFistOn(bool Toggle) {
		if (!this) { return; }

		uint64_t playerinfo = client.read<uint64_t>(reinterpret_cast<uintptr_t>(this) + offsets::m_PlayerInfo);
		//Mem.Write<float>(playerinfo + 0x0D6C, 1000000.0f);
	}
	void SuperFistOff(bool Toggle) {
		if (!this) { return; }

		uint64_t playerinfo = client.read<uint64_t>(reinterpret_cast<uintptr_t>(this) + offsets::m_PlayerInfo);
		//Mem.Write<float>(playerinfo + 0x0D6C, 1.0f);
	}

	void SetInfCombatRoll(bool enable) {
		if (!this) { return; }

		uintptr_t Address = offsets::m_InfiniteCombatRoll;
		static std::vector<uint8_t> OriginalTable;

		if (OriginalTable.empty()) {
		//	OriginalTable = client.readBytes(Address, 6);
		}

		std::vector <uint8_t> Patch = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };

		//Mem.WriteBytes(Address, enable ? Patch : OriginalTable);
	}

	D3DXVECTOR3 GetBonePosDefault(const int Bone)
	{
		if (!this) { return D3DXVECTOR3(0, 0, 0); }
		D3DXMATRIX Mtx = client.read<D3DXMATRIX>(reinterpret_cast<uintptr_t>(this) + 0x60);
		D3DXVECTOR3 BonePos = client.read<D3DXVECTOR3>(reinterpret_cast<uintptr_t>(this) + (0x410 + Bone * 0x10));;

		D3DXVECTOR4 Transform;
		D3DXVec3Transform(&Transform, &BonePos, &Mtx);
		return D3DXVECTOR3(Transform.x, Transform.y, Transform.z);
	}

	float GetDistance(D3DXVECTOR3 pos1, D3DXVECTOR3 pos2) {
		return std::sqrtf(std::pow(pos2.x - pos1.x, 2.f) + std::pow(pos2.y - pos1.y, 2.f));
	}

	uintptr_t GetCPedInventory() {
		if (!this) { return 0; }
		return client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + (offsets::m_WeaponManager - 8));
	}

	uintptr_t CurrentWeapon() {
		if (!this) { return 0; }

		return client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + (offsets::m_WeaponManager + 0x20));
	}

	uintptr_t GetNavigation() {
		if (!this) { return 0; }
		return client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x30);
	}

	void RemoveKinematics()
	{
		if (!offsets::m_ArmsKinematics || !offsets::m_LegsKinematics)
			return;

	//	Mem.PatchFunc(offsets::m_ArmsKinematics, 5);
		//Mem.PatchFunc(offsets::m_LegsKinematics, 5);
	}

	void ForceWeaponWheel(bool toggle)
	{
		if (!this) return;

		static uintptr_t DisableControlAction = 0;
		static uintptr_t HideHudComponentThisFrame = 0;

		/*if (DisableControlAction == 0) {
			DisableControlAction = Mem.FindSignature(
				{ 0x48, 0x8b, 0x41, 0x00 , 0x83, 0x78, 0x00 , 0x00 , 0x8b, 0x50, 0x00 , 0x8b, 0x08, 0xe9, 0x00 , 0x00 , 0x00 , 0x00 , 0x48, 0x89, 0x5c, 0x24 }
			);
		}

		if (HideHudComponentThisFrame == 0) {
			HideHudComponentThisFrame = Mem.FindSignature(
				{ 0x48, 0x83, 0xec, 0x00 , 0x48, 0x8b, 0x41, 0x00 , 0x83, 0x38, 0x00 , 0x48, 0x89, 0x6c, 0x24 }
			);
		}*/

		if (toggle) {
			//Mem.WriteBytes(DisableControlAction, { 0xC3 });
			//Mem.WriteBytes(HideHudComponentThisFrame, { 0xC3 });
			SetConfigFlag(BlockWeaponSwitching, false);
		}
		else {
			//Mem.WriteBytes(DisableControlAction, { 0x48 });
			//Mem.WriteBytes(HideHudComponentThisFrame, { 0x48 });
			SetConfigFlag(BlockWeaponSwitching, true); //Isso pode dar problema pra frente*
		}
	}

};

class CHandlingData {
public:

	uint64_t qword0; //0x0000
	uint32_t m_model_hash; //0x0008
	float m_mass; //0x000C
	float m_initial_drag_coeff; //0x0010
	float m_downforce_multiplier; //0x0014
	float m_popup_light_rotation; //0x0018
	char pad_001C[4]; //0x001C
	D3DXVECTOR3 m_centre_of_mass; //0x0020
	char pad_002C[4]; //0x002C
	D3DXVECTOR3 m_inertia_mult; //0x0030
	char pad_003C[4]; //0x003C
	float m_buoyancy; //0x0040
	float m_drive_bias_rear; //0x0044
	float m_drive_bias_front; //0x0048
	float m_acceleration; //0x004C
	uint8_t m_initial_drive_gears; //0x0050
	char pad_0051[3]; //0x0051
	float m_drive_inertia; //0x0054
	float m_upshift; //0x0058
	float m_downshift; //0x005C
	float m_initial_drive_force; //0x0060
	float m_drive_max_flat_velocity; //0x0064
	float m_initial_drive_max_flat_vel; //0x0068
	float m_brake_force; //0x006C
	char pad_0070[4]; //0x0070
	float m_brake_bias_front; //0x0074
	float m_brake_bias_rear; //0x0078
	float m_handbrake_force; //0x007C
	float m_steering_lock; //0x0080
	float m_steering_lock_ratio; //0x0084
	float m_traction_curve_max; //0x0088
	float m_traction_curve_lateral; //0x008C
	float m_traction_curve_min; //0x0090
	float m_traction_curve_ratio; //0x0094
	float m_curve_lateral; //0x0098
	float m_curve_lateral_ratio; //0x009C
	float m_traction_spring_delta_max; //0x00A0
	float m_traction_spring_delta_max_ratio; //0x00A4
	float m_low_speed_traction_loss_mult; //0x00A8
	float m_camber_stiffness; //0x00AC
	float m_traction_bias_front; //0x00B0
	float m_traction_bias_rear; //0x00B4
	float m_traction_loss_mult; //0x00B8
	float m_suspension_force; //0x00BC
	float m_suspension_comp_damp; //0x00C0
	float m_suspension_rebound_damp; //0x00C4
	float m_suspension_upper_limit; //0x00C8
	float m_suspension_lower_limit; //0x00CC
	float m_suspension_raise; //0x00D0
	float m_suspension_bias_front; //0x00D4
	float m_suspension_bias_rear; //0x00D8
	float m_anti_rollbar_force; //0x00DC
	float m_anti_rollbar_bias_front; //0x00E0
	float m_anti_rollbar_bias_rear; //0x00E4
	float m_roll_centre_height_front; //0x00E8
	float m_roll_centre_height_rear; //0x00EC
	float m_collision_damage_mult; //0x00F0
	float m_weapon_damamge_mult; //0x00F4
	float m_deformation_mult; //0x00F8
	float m_engine_damage_mult; //0x00FC
	float m_petrol_tank_volume; //0x0100
	float m_oil_volume; //0x0104
	char pad_0108[4]; //0x0108
	D3DXVECTOR3 m_seat_offset_dist; //0x010C
	uint32_t m_monetary_value; //0x0118
	char pad_011C[8]; //0x011C
	uint32_t m_model_flags; //0x0124
	uint32_t m_handling_flags; //0x0128
	uint32_t m_damage_flags; //0x012C
	char pad_0130[12]; //0x0130
	uint32_t m_ai_handling_hash; //0x013C
	char pad_140[24]; //0x140
};

class CVehicle {
public:

	D3DXVECTOR3 GetPos() {
		if (!this) { return D3DXVECTOR3(0, 0, 0); }
		return client.read<D3DXVECTOR3>(reinterpret_cast<uintptr_t>(this) + 0x90);
	}

	void SetPos(D3DXVECTOR3 Pos) {
		if (!this) { return; }

		//suintptr_t Navigation = client.read<uintptr_t>( reinterpret_cast< uintptr_t >( this ) + 0x30 );
		////Mem.Write<D3DXVECTOR3>( Navigation + 0x30, D3DXVECTOR3( 0, 0, 0 ) );
		//Mem.Write<D3DXVECTOR3>(reinterpret_cast<uintptr_t>(this) + 0x90, Pos);
	}

	uintptr_t GetHandling() {
		if (!this) { return 0; }
		return client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + offsets::m_Handling);
	}

	bool GetGodMode() {
		if (!this) { return false; }
		//Atualizar Offsets ??
		return client.read<BYTE>(reinterpret_cast<uintptr_t>(this) + 0x189);
	}

	void SetGodMode(bool Toggle) {
		if (!this) { return; }
		//Atualizar Offsets ??
		//Mem.Write<BYTE>(reinterpret_cast<uintptr_t>(this) + 0x189, Toggle);
	}

	float GetGravity() {
		if (!this) { return 0; }
		return client.read<float>(reinterpret_cast<uintptr_t>(this) + offsets::m_VehicleGravity);
	}

	void SetGravity(float Value) {
		if (!this) { return; }
		//Mem.Write<float>(reinterpret_cast<uintptr_t>(this) + offsets::m_VehicleGravity, Value);
	}

	void Fix() {
		if (!this) { return; }

		//Mem.Write<uint32_t>(reinterpret_cast<uintptr_t>(this) + offsets::m_VehicleState, (BYTE)0x17);
	}

	bool IsLocked() {
		if (!this) { return false; }
		return client.read<uint32_t>(reinterpret_cast<uintptr_t>(this) + offsets::m_VehicleDoorsLockState) == 2;
	}

	void DoorState(bool Unlock) {
		if (!this) { return; }

		//Mem.Write<uint32_t>(reinterpret_cast<uintptr_t>(this) + offsets::m_VehicleDoorsLockState, Unlock ? 1 : 2);
	}

	D3DXVECTOR3 GetVelocity() {
		if (!this) { return D3DXVECTOR3(0, 0, 0); }
		return client.read<D3DXVECTOR3>(reinterpret_cast<uintptr_t>(this) + 0x320);
	}

	CPed* GetDriver()
	{
		if (!this) return 0;
		return (CPed*)client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + offsets::m_VehicleDriver);
	}


};


class CPedFactory {
public:
	CPed* GetLocalPlayer() {
		if (!this) { return 0; }
		return (CPed*)client.read<uintptr_t>(reinterpret_cast<uintptr_t>(this) + 0x8);
	}
};


namespace pointers {
    // Object Pointers
    inline CPedFactory* pWorld = nullptr;
    inline CPed* pLocalPlayer = nullptr;
    inline CReplayInterFace* pReplayInterFace = nullptr;
    inline uintptr_t pBlipList = 0, pViewPort = 0, pCamGamePlayDirector = 0;
}
struct EntityStruct {
	uintptr_t PedAddr;
	int Id;
	int Index;
	int PedType;
	bool IsFriend;
	D3DXVECTOR3 Pos;
	D3DXVECTOR3 HeadBone;
	D3DXVECTOR3 PelvisBone;
	float Health;
	float MaxHealth;
	float Armor;
	float Distance;
};

inline D3DXVECTOR2 WorldToScreen(D3DXVECTOR3 World)
{
	D3DXMATRIX ViewMatrix = client.read<D3DXMATRIX>(pointers::pViewPort + 0x24C);
	D3DXMatrixTranspose(&ViewMatrix, &ViewMatrix);

	auto VecX = D3DXVECTOR4(ViewMatrix._21, ViewMatrix._22, ViewMatrix._23, ViewMatrix._24),
		VecY = D3DXVECTOR4(ViewMatrix._31, ViewMatrix._32, ViewMatrix._33, ViewMatrix._34),
		VecZ = D3DXVECTOR4(ViewMatrix._41, ViewMatrix._42, ViewMatrix._43, ViewMatrix._44);

	D3DXVECTOR3 ScreenPos = D3DXVECTOR3(
		(VecX.x * World.x) + (VecX.y * World.y) + (VecX.z * World.z) + VecX.w,
		(VecY.x * World.x) + (VecY.y * World.y) + (VecY.z * World.z) + VecY.w,
		(VecZ.x * World.x) + (VecZ.y * World.y) + (VecZ.z * World.z) + VecZ.w
	);

	if (ScreenPos.z <= 0.1f)
		return D3DXVECTOR2(0, 0);

	ScreenPos.z = 1.0f / ScreenPos.z;
	ScreenPos.x *= ScreenPos.z;
	ScreenPos.y *= ScreenPos.z;

	// Obter resolução do monitor
	float screenWidth = (float)GetSystemMetrics(SM_CXSCREEN);
	float screenHeight = (float)GetSystemMetrics(SM_CYSCREEN);

	ScreenPos.x += (screenWidth / 2) + float(0.5f * ScreenPos.x * screenWidth + 0.5f);
	ScreenPos.y = (screenHeight / 2) - float(0.5f * ScreenPos.y * screenHeight + 0.5f);

	return D3DXVECTOR2(ScreenPos.x, ScreenPos.y);
}

inline bool IsOnScreen(D3DXVECTOR2 Pos)
{
	if (Pos == D3DXVECTOR2(0, 0)) return false;
	return true;
}


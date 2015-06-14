/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_SERVER_ENTITIES_CHARACTER_H
#define GAME_SERVER_ENTITIES_CHARACTER_H

#include <game/server/entity.h>
#include <game/generated/server_data.h>
#include <game/generated/protocol.h>

#include <game/gamecore.h>

#include <game/server/upgradelist.h>

enum
{
	WEAPON_GAME = -3, // team switching etc
	WEAPON_SELF = -2, // console kill command
	WEAPON_WORLD = -1, // death tiles etc
};

class CCharacter : public CEntity
{
	MACRO_ALLOC_POOL_ID()

public:
	//character's size
	static const int ms_PhysSize = 28;

	CCharacter(CGameWorld *pWorld);

	virtual void Reset();
	virtual void Destroy();
	virtual void Tick();
	virtual void TickDefered();
	virtual void TickPaused();
	virtual void Snap(int SnappingClient);

	bool IsGrounded();

	void HandleWeaponSwitch();
	void DoWeaponSwitch();

	void HandleWeapons();
	void HandleNinja();

	void OnPredictedInput(CNetObj_PlayerInput *pNewInput);
	void OnDirectInput(CNetObj_PlayerInput *pNewInput);
	void ResetInput();
	void FireWeapon();

	void Die(int Killer, int Weapon, bool SkipKillMessage = false);
	bool TakeDamage(vec2 Force, int Dmg, int From, int Weapon);
	void TakeDeathtileDamage();

	bool Spawn(class CPlayer *pPlayer, vec2 Pos);
	bool Remove();
	
	void Teleport(vec2 Pos);

	bool IncreaseHealth(int Amount);
	bool IncreaseArmor(int Amount);
	
	void SetHealth(int Health);

	void GiveNinja();

	void SetEmote(int Emote, int Tick);
	void SetEmoteFor(int Emote, int Ticks, int LockEmote = 0, bool UseTime = true);

	bool IsAlive() const { return m_Alive; }
	class CPlayer *GetPlayer() { return m_pPlayer; }

	bool HookGrabbed();
	
	bool m_IsBot;
	int m_HiddenHealth;
	int m_MaxHealth;
	
	bool m_WeaponPicked;
	
	int GetActiveWeapon(){ return m_ActiveWeapon; }
	
	
	int m_DeathTileTimer;


	void ForceNogo();
	bool InNogo();

	void Explode(int Size = 1);

	// boss "giant"
	CCharacter *m_Bottom;
	CCharacter *m_Top;
	
	void TopTick();
	
	
	// custom weapon system
	int m_ActiveCustomWeapon;
	int m_ClipReloadTimer;
	
	struct CustomWeaponStat
	{
		int m_Ammo;
		int m_AmmoReserved; // ammos left in clips
		bool m_Got;
		bool m_Disabled;
		bool m_Ready;
	} m_aWeapon[NUM_CUSTOMWEAPONS];
	
	bool AddClip();
	
	bool GotWeapon(int CustomWeapon){ return m_aWeapon[CustomWeapon].m_Got; }
	bool WeaponDisabled(int CustomWeapon){ return m_aWeapon[CustomWeapon].m_Disabled; }
	void DisableWeapon(int CustomWeapon){ m_aWeapon[CustomWeapon].m_Disabled = true; }
	
	bool GiveCustomWeapon(int CustomWeapon);
	void SetCustomWeapon(int CustomWeapon);
	
	void GiveClassWeapon();
	
	void DoReloading(bool SkipAmmoCheck = false);
	void ShowArmor();
	
	void ScanWeapons();
	int GetFirstWeapon(int ParentType);
	int GetWeapon(int ParentType);
	int GetNextWeapon(int ParentType);
	int GetPrevWeapon(int ParentType);
	
	// next that shares a parent
	int m_aNextWeapon[NUM_WEAPONS];
	
	// not in use
	int m_Grenades;
	void AddGrenades(int Num);
	void ThrowGrenade();
	
	
	// class stuff
	int m_ClassAbilityTimer;
	
	int m_Bloodlust;
	int m_BloodlustSoundTimer;
	
	void ActivateBloodlust();
	
	bool SetLandmine();
	bool SetElectromine();

	bool m_GodMode;
	
	int m_CryTimer;
	int m_CryState;
	
	void Cry();
	
	
	CCharacterCore GetCore(){ return m_Core; }
	vec2 GetPosition(){ return m_Pos; }
	
	
private:
	// player controlling this character
	class CPlayer *m_pPlayer;

	bool m_Alive;
	
	CCharacter *m_LockedTo;
	
	// weapon info
	CEntity *m_apHitObjects[10];
	int m_NumObjectsHit;

	int m_ActiveWeapon;
	int m_LastWeapon;
	int m_QueuedCustomWeapon;

	int m_ReloadTimer;
	int m_AttackTick;

	int m_DamageTaken;

	int m_EmoteType;
	int m_EmoteStop;
	int m_EmoteLockStop;

	// last tick that the player took any action ie some input
	int m_LastAction;
	int m_LastNoAmmoSound;
	
	int m_LastReloadingSound;

	// these are non-heldback inputs
	CNetObj_PlayerInput m_LatestPrevInput;
	CNetObj_PlayerInput m_LatestInput;

	// input
	CNetObj_PlayerInput m_PrevInput;
	CNetObj_PlayerInput m_Input;
	int m_NumInputs;
	int m_Jumped;

	int m_DamageTakenTick;

	int m_Health;
	int m_Armor;

	bool m_DelayedKill;
	
	// ninja
	struct
	{
		vec2 m_ActivationDir;
		int m_ActivationTick;
		int m_CurrentMoveTime;
		int m_OldVelAmount;
	} m_Ninja;

	// the player core for the physics
	CCharacterCore m_Core;

	// info for dead reckoning
	int m_ReckoningTick; // tick that we are performing dead reckoning From
	CCharacterCore m_SendCore; // core that we should send
	CCharacterCore m_ReckoningCore; // the dead reckoning core

};

#endif

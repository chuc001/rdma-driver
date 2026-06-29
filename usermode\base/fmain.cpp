#include "includes.h"
#define KMBOX_IP "192.168.2.195"
#define KMBOX_PORT "5117"
#define KMBOX_MAC "13F65054"
int main()
{
	if (!memory_init())
	{
		return 1;
	}
	//offsets::m_World = client.base_addres_a + 0x2593320;
	//offsets::m_ReplayInterFace = client.base_addres_a + 0x1F58B58;
	//offsets::m_ViewPort = client.base_addres_a + 0x20019E0;
	//offsets::m_CamGameplayDirector = client.base_addres_a + 0x2002888;
	//offsets::m_BlipList = client.base_addres_a + 0x2002FA0;
	//offsets::m_MaxHealth = 0x284;
	//offsets::m_WeaponManager = 0x10B8;
	//offsets::m_EntityType = 0x1098;
	//offsets::m_LastVehicle = 0xD10;
	//offsets::m_PlayerInfo = 0x10A8;
	//offsets::m_FragInst = 0x1430;
	//offsets::m_PlayerId = 0xE8;
	//offsets::m_Armor = 0x150C;
	//offsets::m_PedFlag = 0x1444;
	//offsets::m_VehicleState = 0x972;
	//offsets::m_FrameFlag = 0x0270;

	offsets::m_World = client.base_addres_a  + 0x25b14b0;;
	offsets::m_ReplayInterFace = client.base_addres_a  + 0x1FBD4F0;
	offsets::m_ViewPort = client.base_addres_a  + 0x201dba0;;
	offsets::m_CamGameplayDirector = client.base_addres_a  + 0x201ED50;
	offsets::m_BlipList = client.base_addres_a  + 0x2023400;

	offsets::m_MaxHealth = 0x284;
	offsets::m_WeaponManager = 0x10B8;
	offsets::m_EntityType = 0x1098;
	offsets::m_LastVehicle = 0xD10;
	offsets::m_PlayerInfo = 0x10A8;
	offsets::m_FragInst = 0x1430;
	offsets::m_PlayerId = 0xE8;
	offsets::m_Armor = 0x150C;
	offsets::m_PedFlag = 0x1444;
	offsets::m_VehicleState = 0x972;
	offsets::m_FrameFlag = 0x0270;


	std::cout << "offsets::m_World 0x" << std::hex << offsets::m_World << std::endl;
	std::cout << "offsets::m_ReplayInterFace 0x" << std::hex << offsets::m_ReplayInterFace << std::endl;
	std::cout << "offsets::m_ViewPort 0x" << std::hex << offsets::m_ViewPort << std::endl;
	std::cout << "offsets::m_CamGameplayDirector 0x" << std::hex << offsets::m_CamGameplayDirector << std::endl;
	std::cout << "offsets::m_BlipList 0x" << std::hex << offsets::m_BlipList << std::endl;
	kmNet_init((char*)KMBOX_IP, (char*)KMBOX_PORT, (char*)KMBOX_MAC);
	kmNet_monitor(1); // Habilita o modo de monitoramento

	//std::thread(update_pointer).detach();
	//std::thread(Core::Threads::Update).detach();
	std::thread(aimbot_thread).detach();

	CreateOverlayWindow();
	directx();
	render_loop();

	system("pause");
	return 0;
}
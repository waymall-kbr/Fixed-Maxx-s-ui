#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>  
#include <Windows.h>
#include <string>
#include <cassert>
#include <emmintrin.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <signal.h>
#include "xorstr.h"
#include "jopa.h"
#include "Math.h"
#include <unordered_map>



int GetProcessIdByName(const char* processname, bool debug = false)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	DWORD result = NULL;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hProcessSnap) return(FALSE);

	pe32.dwSize = sizeof(PROCESSENTRY32); // <----- IMPORTANT

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);          // clean the snapshot object
		if (debug)
			printf("Failed to gather information on system processes! \n");
		return(NULL);
	}

	do
	{
		if (0 == strcmp(processname, pe32.szExeFile))
		{
			result = pe32.th32ProcessID;
			break;
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);

	return result;
}

typedef struct _UncStr
{
	char stub[0x10];
	int len;
	wchar_t str;
} *pUncStr;

using namespace std;


uint64_t gBase, uBase;
int pid;
SOCKET Sock;
long long firstentry = 0;
UINT64 TodCycle = 0;


inline HANDLE DriverHandle;
inline HWND hwnd = NULL;
inline DWORD processID;

inline int wLeft, wTop;

#define CHECK_VALID( _v ) 0
#define Assert( _exp ) ((void)0)

#define FastSqrt(x)			(sqrt)(x)

#define M_PI 3.14159265358979323846264338327950288419716939937510


typedef struct _NULL_MEMORY
{
	void* buffer_address;
	UINT_PTR address;
	ULONGLONG size;
	ULONG pid;
	BOOLEAN write;
	BOOLEAN read;
	BOOLEAN req_base;
	void* output;
	const char* module_name;
	ULONG64 base_address;
	DWORD64 GA;
	DWORD64 UP;
}NULL_MEMORY;






struct HandleDisposer
{
	using pointer = HANDLE;
	void operator()(HANDLE handle) const
	{
		if (handle != NULL || handle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(handle);
		}
	}
};

inline uintptr_t oBaseAddress = 0;
using unique_handle = std::unique_ptr<HANDLE, HandleDisposer>;

template<typename ... Arg>
inline uint64_t CallHook(const Arg ... args)
{
	void* hooked_func = GetProcAddress(LoadLibrary("win32u.dll"), "NtGdiDdDDINetDispGetNextChunkInfo");

	auto func = static_cast<uint64_t(_stdcall*)(Arg...)>(hooked_func);

	return func(args ...);
}

inline static ULONG64 GetModuleBaseAddress(const char* module_name)
{
	NULL_MEMORY instructions = { 0 };
	instructions.pid = pid;
	instructions.req_base = TRUE;
	instructions.read = FALSE;
	instructions.write = FALSE;
	instructions.module_name = module_name;
	CallHook(&instructions);
	gBase = instructions.GA;
	uBase = instructions.UP;
	std::cout << "GA : " << instructions.GA << std::endl;
	std::cout << "UP : " << instructions.UP << std::endl;
	ULONG64 base = NULL;
	base = instructions.base_address;
	return base;
}

template <typename Type>
inline Type read(unsigned long long int Address)
{
	Type response{};
	NULL_MEMORY instructions;
	instructions.pid = pid;
	instructions.size = sizeof(Type);
	instructions.address = Address;
	instructions.read = TRUE;
	instructions.write = FALSE;
	instructions.req_base = FALSE;
	instructions.output = &response;
	CallHook(&instructions);

	return response;
}


template <typename Type>
inline Type readmem(unsigned long long int Address, int len)
{
	Type response{};
	NULL_MEMORY instructions;
	instructions.pid = pid;
	instructions.size = len;
	instructions.address = Address;
	instructions.read = TRUE;
	instructions.write = FALSE;
	instructions.req_base = FALSE;
	instructions.output = &response;
	CallHook(&instructions);

	return response;
}

inline void writefloat(unsigned long long int Address, float stuff)
{
	NULL_MEMORY instructions;
	instructions.address = Address;
	instructions.pid = pid;
	instructions.write = TRUE;
	instructions.read = FALSE;
	instructions.req_base = FALSE;
	instructions.buffer_address = (void*)&stuff;
	instructions.size = sizeof(float);

	CallHook(&instructions);
}


inline bool writevall(unsigned long long int Address, UINT_PTR value, SIZE_T write_size)
{
	NULL_MEMORY instructions;
	instructions.address = Address;
	instructions.pid = pid;
	instructions.write = TRUE;
	instructions.read = FALSE;
	instructions.req_base = FALSE;
	instructions.buffer_address = (void*)value;
	instructions.size = write_size;

	CallHook(&instructions);
	return true;
}
template<typename S>
inline bool writeval(UINT_PTR write_address, const S& value)
{
	return writevall(write_address, (UINT_PTR)&value, sizeof(S));
}


inline void writedouble(unsigned long long int Address, double stuff)
{
	NULL_MEMORY instructions;
	instructions.address = Address;
	instructions.pid = pid;
	instructions.write = TRUE;
	instructions.read = FALSE;
	instructions.req_base = FALSE;
	instructions.buffer_address = (void*)&stuff;
	instructions.size = sizeof(double);

	CallHook(&instructions);
}

#pragma once
#include <Windows.h>

#define safe_read(Addr, Type) read<Type>(Addr)
#define safe_write(Addr, Data, Type) writeval<Type>(Addr, Data)
#define safe_memcpy(Dst, Src, Size) driver::write_memory(Sock, pid, Dst, driver::read_memory(Sock, pid, Src, 0, Size), Size)

int length(uintptr_t addr) { return safe_read(addr + 0x10, int); }

std::string readstring(uintptr_t addr) {
	try {
		static char buff[128] = { 0 };
		buff[length(addr)] = '\0';

		for (int i = 0; i < length(addr); ++i) {
			if (buff[i] < 128) {
				buff[i] = safe_read(addr + 0x14 + (i * 2), char);
			}
			else {
				buff[i] = '?';
				if (buff[i] >= 0xD800 && buff[i] <= 0xD8FF)
					i++;
			}
		}
		return std::string(&buff[0], &buff[length(addr)]);
	}
	catch (const std::exception& exc) {}
	return "Error";
}
struct monostr
{
	char buffer[128];
};
std::string readchar(uintptr_t addr) {
	std::string str = read<monostr>(addr).buffer;
	if (!str.empty())
		return str;
	else
		return (std::string)("31");
}


#include "Of.h"

#include "Main.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_internal.h"
#include "Overlay.h"
#include "Draw.h"

LPCSTR TargetTitle = "Rust";
LPCSTR OverName = "Rust External - Client";
bool CreateConsole = true;// true false

#include "Value.h"
#include "SDK/Math.h"
#include "SDK/BaseProjectile.h"
#include "SDK/BasePlayer.h"
#include "SDK/Misc.h"
#include "SDK/Aim.h"


BasePlayer* AimEntity = nullptr;
BasePlayer localclass;
BasePlayer currentent;
BasePlayer currentplayer;
BaseProjectile wep;
//bool OnServer;

#include <mutex>
inline std::mutex          entity_mutex;
inline std::vector<BasePlayer*> otherplayers;
inline std::vector<Vector3> Stash;
inline std::vector<Vector3> hemp;
inline std::vector<Vector3> backpack;
inline std::vector<Vector3> corpse;
inline std::vector<Vector3> vehicles;
inline std::vector<Vector3> DroppedItem;
inline std::vector<Vector3> Airdrop;
inline std::vector<Vector3> patrol_heli;
inline std::vector<Vector3> hackable_crate;
inline std::vector<Vector3> high_tier_crates;
inline std::vector<Vector3> low_tier_crates;
inline std::vector<Vector3> SulfurNodes;
inline std::vector<Vector3> StoneNodes;
inline std::vector<Vector3> MetalNodes;

RGBA Grey = { 180, 180, 180, 255 };


void WeaponFix(BaseProjectile* weapon)
{
	int ItemID = 0;
	if (weapon)
		ItemID = weapon->GetItemID();
	if (!ItemID) {
		//printf("No weapon found\n");
		return;
	}

	auto WeaponString = LocalPlayer.BasePlayer->GetActiveWeaponcChars();
	//printf("WeaponString: %s\n", WeaponString.c_str());

	char LPWeapon[64];
	sprintf(LPWeapon, "Activate weapon : %s", WeaponString.c_str());
	//printf("LPWeapon: %s\n", LPWeapon);

	ImVec2 textSize = ImGui::CalcTextSize(LPWeapon);
	//..printf("Text size: x=%.2f, y=%.2f\n", textSize.x, textSize.y);

	DrawStrokeText((Value::floats::Screen::W / 2) - (textSize.x / 2), 15, &Grey, LPWeapon);
	if (Value::bools::Aim::SilentAim) {
		SilentAim(LocalPlayer.BasePlayer);
	}
	for (const char* Name : Value::WeaponName::SemiAutomatic) {
		if (WeaponString.find(Name) != std::string::npos) {
			weapon->NoRecoil();
			weapon->NoSpread();
			weapon->FatBullet();
			weapon->SetAutomatic();
			return;
		}
	}
	
	for (const char* Name : Value::WeaponName::Sniper) {
		if (WeaponString.find(Name) != std::string::npos) {
			weapon->NoRecoil();
			weapon->NoSpread();
			weapon->FatBullet();
			return;
		}
	}
	for (const char* Name : Value::WeaponName::Automatic) {
		if (WeaponString.find(Name) != std::string::npos) {
			weapon->NoRecoil();
			weapon->NoSpread();
			weapon->FatBullet();
			return;
		}
	}
	for (const char* Name : Value::WeaponName::Bow) {
		if (WeaponString.find(Name) != std::string::npos) {
			weapon->FatBullet();
			return;
		}
	}
}



bool mfound = false;
void InitLocalPlayer() {

	long long i = 0;
	UINT64  ObjMgr = safe_read(uBase + oGameObjectManager, UINT64);
	if (!ObjMgr)
		return;
	UINT64  Obj = safe_read(ObjMgr + 0x8, UINT64);
	if (!Obj)
		return;
	bool LP_isValid = false;

	if (!ObjMgr) return;
	UINT64 end = safe_read(ObjMgr, UINT64);
	for (UINT64 Obj = safe_read(ObjMgr + 0x8, UINT64); (Obj && (Obj != end)); Obj = safe_read(Obj + 0x8, UINT64))// 0x8 0x18
	{
		UINT64 GameObject = safe_read(Obj + 0x10, UINT64);
		WORD Tag = safe_read(GameObject + 0x54, WORD);

		if (Tag == 6 || Tag == 5 || Tag == 20011)
		{
			UINT64 ObjClass = safe_read(GameObject + 0x30, UINT64);
			UINT64 Entity = safe_read(ObjClass + 0x18, UINT64);
			if (Tag == 5)
			{
				UINT64 ObjClass = safe_read(GameObject + 0x30, UINT64);
				UINT64	Entity = safe_read(ObjClass + 0x18, UINT64);
				LocalPlayer.pViewMatrix = (Matrix4x4*)(Entity + 0x2E4);// 0xDC 0x2E4
				printf("Found matrix!\n");
				mfound = true;
				return;
			}

			else if (Tag == 20011)
			{
				UINT64 ObjClass = safe_read(GameObject + 0x30, UINT64);
				UINT64	Entity = safe_read(ObjClass + 0x18, UINT64);
				UINT64 Dome = safe_read(Entity + 0x28, UINT64);
				TodCycle = safe_read(Dome + 0x38, UINT64);
			}
		}
	NextEnt:
		continue;
	}

	if (!TodCycle || !LP_isValid || LocalPlayer.BasePlayer->IsMenu()) {
		return;
	}
}

void InputHandler() {
	for (int i = 0; i < 5; i++) ImGui::GetIO().MouseDown[i] = false;
	int button = -1;
	if (GetAsyncKeyState(VK_LBUTTON)) button = 0;
	if (button != -1) ImGui::GetIO().MouseDown[button] = true;
}
void Draw() {
	char fpsinfo[64];
	sprintf(fpsinfo, "%s", "Rust External - Client");
	DrawStrokeText(15, Value::floats::Screen::H - 25, &Grey, fpsinfo);

	if (Value::bools::Aim::Fov) {
		RGBA FovColor = { Value::Colors::Aim::Fov[0] * 255, Value::Colors::Aim::Fov[1] * 255 , Value::Colors::Aim::Fov[2] * 255, Value::Colors::Aim::Fov[3] * 255 };
		DrawCircle(Value::floats::Screen::W / 2, Value::floats::Screen::H / 2, Value::floats::Aim::Fov, &FovColor, 50);
	}
	if (Value::bools::Visuals::Radar::Enable) {
		RGBA Ext = { 255, 255, 255, 255 };
		RGBA Int = { 180, 180, 180, 50 };
		DrawCircle(Value::floats::Screen::W - Value::floats::Visuals::Radar::Radius, Value::floats::Visuals::Radar::Radius, Value::floats::Visuals::Radar::Radius, &Ext, 50);
		DrawCircleFilled(Value::floats::Screen::W - Value::floats::Visuals::Radar::Radius, Value::floats::Visuals::Radar::Radius, Value::floats::Visuals::Radar::Radius, &Int);
	}
}
#include <comdef.h>
Vector2 Penis;

inline void RadarPlayer(BasePlayer* player)
{
	if (LocalPlayer.BasePlayer && Value::bools::Visuals::Radar::Enable)
	{
		if (!player->IsDead() && player->GetHealth() >= 0.2f)
		{
			const Vector3 LocalPos = LocalPlayer.BasePlayer->GetBoneByID();
			const Vector3 PlayerPos = player->GetBoneByID();
			const float Distance = (int)Math::Calc3D_Dist(LocalPos, PlayerPos);
			const float y = LocalPos.x - PlayerPos.x;
			const float x = LocalPos.z - PlayerPos.z;
			Vector2 LocalEulerAngles = LocalPlayer.BasePlayer->GetVA();

			const float num = atan2(y, x) * 57.29578f - 270.f - LocalEulerAngles.y;
			float PointPos_X = Distance * cos(num * 0.0174532924f);
			float PointPos_Y = Distance * sin(num * 0.0174532924f);
			PointPos_X = (PointPos_X * ((Value::floats::Visuals::Radar::Radius * 2) / Value::floats::Visuals::Radar::Distance)) / 2.f;
			PointPos_Y = (PointPos_Y * ((Value::floats::Visuals::Radar::Radius * 2) / Value::floats::Visuals::Radar::Distance)) / 2.f;
			if (!player->HasFlags(16))
			{
				if (Distance <= Value::floats::Visuals::Radar::Distance)
				{
					RGBA Red = { 255, 0, 0, 255 };
					DrawCircleFilled(Value::floats::Screen::W - Value::floats::Visuals::Radar::Radius + PointPos_X, Value::floats::Visuals::Radar::Radius + PointPos_Y, 2, &Red);
					//GUI::Render.FillCircle({ pos.x + Vars::Radar::Radar_Size / 2.f + PointPos_X - 3.f, pos.y + Vars::Radar::Radar_Size / 2.f + PointPos_Y - 3.f }, D2D1::ColorF::Lime, 3.f);
				}
			}
			else
				if (player->HasFlags(16) && Value::bools::Visuals::Radar::Sleeper)
				{
					if (Distance <= Value::floats::Visuals::Radar::Distance)
					{

						//Value::floats::Screen::W - Value::floats::Visuals::Radar::Radius, Value::floats::Visuals::Radar::Radius
						RGBA Green = { 0, 255, 0, 255 };
						DrawCircleFilled(Value::floats::Screen::W - Value::floats::Visuals::Radar::Radius + PointPos_X, Value::floats::Visuals::Radar::Radius + PointPos_Y, 2, &Green);
						//GUI::Render.FillCircle({ pos.x + Vars::Radar::Radar_Size / 2.f + PointPos_X - 3.f, pos.y + Vars::Radar::Radar_Size / 2.f + PointPos_Y - 3.f }, D2D1::ColorF::Red, 3.f);
					}
				}
		}
	}
}

static const char* Items[] = { "LEFT", "RIGHT", "TOP", "DOWN", "LEFT TOP", "RIGHT TOP", "LEFT DOWN" };

enum Sides
{
	LEFT_S,
	RIGHT_S,
	TOP_S,
	DOWN_S,
	LEFT_TOP_S,
	RIGHT_TOP_S,
	LEFT_DOWN_S,
	RIGHT_DOWN_S,
};

int
NAME = 1,
HEALTH = 1,
BOX = 1,
WEAPON = 1,
DISTANCE = 1;

Vector2
LEFT,
RIGHT,
TOP,
DOWN,
LEFT_TOP,
RIGHT_TOP,
LEFT_DOWN,
RIGHT_DOWN;

struct MoveStruct {
	int Side = LEFT_S;
};

// Структура для хранения данных сглаживания ESP
struct ESPSmoothData {
	UINT64 lastUpdateTime;
	Vector3 lastPosition;  // Объявляем как Vector3
	Vector2 lastScreenPos; // Объявляем как Vector2
	float smoothFactor;
	bool wasVisible;
	bool isInitialized;
};

// Хранилище кэшированных данных для сглаживания
std::unordered_map<UINT64, ESPSmoothData> g_ESPSmoothCache;

// Функция для сглаживания векторов
Vector3 SmoothVector3(const Vector3& current, const Vector3& target, float smoothFactor) {
	return Vector3(
		current.x + (target.x - current.x) * smoothFactor,
		current.y + (target.y - current.y) * smoothFactor,
		current.z + (target.z - current.z) * smoothFactor
	);
}
// Структура для хранения точных габаритов модели игрока
struct PlayerBounds {
	Vector2 topLeft;
	Vector2 bottomRight;
	float height;
	float width;
	bool isValid;
};
// Улучшенная функция для определения точных границ модели игрока
PlayerBounds CalculatePlayerBounds(BasePlayer* player, BasePlayer* localPlayer) {
	PlayerBounds bounds;
	bounds.isValid = false;

	if (!player || !localPlayer) return bounds;

	// Получаем кости для более точного определения габаритов
	Vector3 head = player->GetBoneByID2(BoneList::head);
	Vector3 neck = player->GetBoneByID2(BoneList::neck);
	Vector3 pelvis = player->GetBoneByID2(BoneList::pelvis);
	Vector3 lFoot = player->GetBoneByID2(BoneList::l_foot);
	Vector3 rFoot = player->GetBoneByID2(BoneList::r_foot);
	Vector3 lShoulder = player->GetBoneByID2(BoneList::l_upperarm);
	Vector3 rShoulder = player->GetBoneByID2(BoneList::r_upperarm);

	// Проверяем валидность основных костей
	if (head.x == 0 && head.y == 0 && head.z == 0 ||
		pelvis.x == 0 && pelvis.y == 0 && pelvis.z == 0) {
		return bounds;
	}

	// Коррекция высоты головы с учетом возможного шлема (+0.2 единицы)
	Vector3 adjustedHead = head;
	adjustedHead.y += 0.2f;

	// Находим самую нижнюю точку (ноги)
	float lowestY = min(lFoot.y, rFoot.y);
	if (lowestY == 0) lowestY = pelvis.y - 1.0f; // Если ноги не определены, аппроксимируем

	// Конвертируем 3D координаты в экранные 2D
	Vector2 headScreen, footScreen, leftScreen, rightScreen;

	// Верхняя точка (голова)
	if (!LocalPlayer.WorldToScreen(adjustedHead, headScreen)) return bounds;

	// Нижняя точка (ноги)
	Vector3 feetPos = Vector3(pelvis.x, lowestY, pelvis.z);
	if (!LocalPlayer.WorldToScreen(feetPos, footScreen)) return bounds;

	// Левое плечо
	if (!LocalPlayer.WorldToScreen(lShoulder, leftScreen)) {
		// Если не удалось получить плечо, аппроксимируем на основе таза
		Vector3 leftApprox = Vector3(pelvis.x - 0.5f, pelvis.y, pelvis.z);
		if (!LocalPlayer.WorldToScreen(leftApprox, leftScreen)) return bounds;
	}

	// Правое плечо
	if (!LocalPlayer.WorldToScreen(rShoulder, rightScreen)) {
		// Если не удалось получить плечо, аппроксимируем на основе таза
		Vector3 rightApprox = Vector3(pelvis.x + 0.5f, pelvis.y, pelvis.z);
		if (!LocalPlayer.WorldToScreen(rightApprox, rightScreen)) return bounds;
	}

	// Расчет ширины бокса на основе расстояния между плечами
	float shoulderWidth = abs(leftScreen.x - rightScreen.x);

	// Корректировка ширины в зависимости от расстояния до игрока
	float distance = Math::Calc3D_Dist(localPlayer->position, player->position);
	float widthAdjustFactor = 1.2f; // Настраиваемый коэффициент для ширины бокса

	if (distance < 10.0f) {
		// Вблизи делаем бокс немного шире
		widthAdjustFactor = 1.4f;
	}
	else if (distance > 50.0f) {
		// Вдалеке немного уже
		widthAdjustFactor = 1.0f;
	}

	float boxWidth = shoulderWidth * widthAdjustFactor;

	// Расчет высоты бокса
	float boxHeight = abs(headScreen.y - footScreen.y);

	// Рассчитываем центр по X и границы бокса
	float centerX = (leftScreen.x + rightScreen.x) / 2.0f;

	// Проверяем, сидит или лежит ли игрок (на основе высоты)
	bool isCrouchedOrSleeping = player->HasFlags(16) || // Флаг спящего
		(boxHeight < shoulderWidth * 1.5f); // Эвристика для сидящих

	if (isCrouchedOrSleeping) {
		// Для сидящих/спящих игроков уменьшаем высоту и корректируем бокс
		boxHeight *= 0.6f;
		footScreen.y = headScreen.y + boxHeight;
	}

	// Заполняем структуру границ
	bounds.topLeft = Vector2(centerX - boxWidth / 2.0f, headScreen.y);
	bounds.bottomRight = Vector2(centerX + boxWidth / 2.0f, footScreen.y);
	bounds.height = boxHeight;
	bounds.width = boxWidth;
	bounds.isValid = true;

	return bounds;
}

// Вспомогательная функция для получения точки на периметре бокса
ImVec2 GetPointOnBoxPerimeter(const PlayerBounds& bounds, float distance) {
	float topWidth = bounds.bottomRight.x - bounds.topLeft.x;
	float leftHeight = bounds.bottomRight.y - bounds.topLeft.y;
	float perimeter = (topWidth + leftHeight) * 2;

	// Нормализуем расстояние к периметру
	while (distance > perimeter) distance -= perimeter;

	// Верхняя сторона
	if (distance < topWidth) {
		return ImVec2(bounds.topLeft.x + distance, bounds.topLeft.y);
	}
	distance -= topWidth;

	// Правая сторона
	if (distance < leftHeight) {
		return ImVec2(bounds.bottomRight.x, bounds.topLeft.y + distance);
	}
	distance -= leftHeight;

	// Нижняя сторона
	if (distance < topWidth) {
		return ImVec2(bounds.bottomRight.x - distance, bounds.bottomRight.y);
	}
	distance -= topWidth;

	// Левая сторона
	return ImVec2(bounds.topLeft.x, bounds.bottomRight.y - distance);
}

void ESP(BasePlayer* BP, BasePlayer* LP) {
	if (Value::bools::Visuals::ESP::Enable) {
		// Сначала применяем уменьшение мерцания костей, если опция включена
		if (Value::bools::Visuals::ESP::ReduceBoneFlicker && BP->IsValid()) {
			// Останавливаем SpineIK, который вызывает мерцание костей
			// Устанавливаем InGesture в true
			UINT64 playerModel = safe_read((uintptr_t)BP + 0x4C0, UINT64); // Оффсет PlayerModel 
			if (playerModel) {
				// Устанавливаем InGesture = true (0x270 - подтвержденный оффсет)
				writeval<bool>(playerModel + 0x270, true);
				
				// Устанавливаем CurrentGesture (0x158 из класса)
				UINT64 currentGesture = safe_read(playerModel + 0x158, UINT64);
				
				// CurrentGestureConfig - нужно заменить на правильный оффсет, если RequiredPlayerGesture
				// имеет аналогичное поле для настройки
				if (currentGesture) {
					// Сохраняем оригинальное значение жеста, если мы первый раз обрабатываем
					static UINT64 originalGesture = 0;
					if (!originalGesture) {
						originalGesture = currentGesture;
					}
					
					// Если есть SleepGesture в классе - используем его, иначе оставляем как есть
					UINT64 sleepGesture = safe_read(playerModel + 0x148, UINT64); // SleepGesture оффсет
					if (sleepGesture) {
						writeval<UINT64>(playerModel + 0x158, sleepGesture); // Установить SleepGesture как текущий
					}
				}
				
				// Отключаем обновление анимации, установив animatorNeedsWarmup в true (0x258)
				writeval<bool>(playerModel + 0x258, true);
			}
		}

		// Отрисовка костей скелета, если включена опция
		if (Value::bools::Visuals::ESP::Bones && BP->IsValid()) {
			RGBA BonesColor = { Value::Colors::Visuals::ESP::Bones[0] * 255, Value::Colors::Visuals::ESP::Bones[1] * 255, Value::Colors::Visuals::ESP::Bones[2] * 255, Value::Colors::Visuals::ESP::Bones[3] * 255 };
			
			// Получаем скорость и направление движения игрока для предсказания позиции
			Vector3 velocity = BP->GetVelocity(); // Получаем скорость игрока
			float predictionFactor = Value::floats::Visuals::ESP::BonePredictionFactor; // Используем настраиваемый коэффициент
			
			// Функция для предсказания позиции кости с учетом движения
			auto PredictBonePosition = [&](BoneList bone) -> Vector3 {
				Vector3 bonePos = BP->GetBoneByID2(bone);
				// Если предсказание включено и игрок движется с достаточной скоростью, предсказываем будущую позицию кости
				if (Value::bools::Visuals::ESP::EnableBonePrediction) {
					float velocityMagnitude = sqrt(velocity.x * velocity.x + velocity.y * velocity.y + velocity.z * velocity.z);
					if (velocityMagnitude > 0.5f) { // Порог скорости
						// Предсказываем будущую позицию кости на основе скорости
						return Vector3(
							bonePos.x + velocity.x * predictionFactor,
							bonePos.y + velocity.y * predictionFactor,
							bonePos.z + velocity.z * predictionFactor
						);
					}
				}
				return bonePos;
			};
			
			// Используем кэш для сглаживания перемещений костей
			// Уникальный ID для игрока (используем адрес)
			UINT64 playerID = (UINT64)BP;
			
			// Получаем или создаем запись кэша для этого игрока
			if (g_ESPSmoothCache.find(playerID) == g_ESPSmoothCache.end()) {
				g_ESPSmoothCache[playerID] = {
					GetTickCount64(),  // Время последнего обновления
					Vector3(),         // Последняя позиция
					Vector2(),         // Последняя экранная позиция
					Value::floats::Visuals::ESP::BoneSmoothingFactor, // Коэффициент сглаживания
					false,             // Был ли виден
					false              // Инициализировано ли
				};
			}
			
			auto& smoothData = g_ESPSmoothCache[playerID];
			smoothData.lastUpdateTime = GetTickCount64();
			// Обновляем коэффициент сглаживания из настроек
			smoothData.smoothFactor = Value::floats::Visuals::ESP::BoneSmoothingFactor;
			
			// Массивы точек скелета
			Vector2 vHead, vNeck, vPelvis, vLShoulder, vRShoulder, vLElbow, vRElbow, vLHand, vRHand, vLKnee, vRKnee, vLFoot, vRFoot;
			
			// Предсказываем и сглаживаем позиции всех костей
			auto SmoothAndProjectBone = [&](BoneList bone, Vector2& screenPos) -> bool {
				// Предсказываем позицию кости
				Vector3 predictedPos = PredictBonePosition(bone);
				
				// Если это первое обновление, просто используем текущую позицию
				if (!smoothData.isInitialized) {
					smoothData.lastPosition = predictedPos;
					smoothData.isInitialized = true;
				}
				
				// Сглаживаем 3D позицию кости
				Vector3 smoothedPos = Vector3(
					smoothData.lastPosition.x + (predictedPos.x - smoothData.lastPosition.x) * smoothData.smoothFactor,
					smoothData.lastPosition.y + (predictedPos.y - smoothData.lastPosition.y) * smoothData.smoothFactor,
					smoothData.lastPosition.z + (predictedPos.z - smoothData.lastPosition.z) * smoothData.smoothFactor
				);
				
				// Обновляем последнюю позицию для следующего кадра
				smoothData.lastPosition = smoothedPos;
				
				// Проецируем на экран
				return LocalPlayer.WorldToScreen(smoothedPos, screenPos);
			};
			
			// Голова - шея
			if (SmoothAndProjectBone(jaw, vHead) && SmoothAndProjectBone(neck, vNeck)) {
				DrawLine(vHead.x, vHead.y, vNeck.x, vNeck.y, &BonesColor, 1.5f);
			}
			
			// Шея - таз
			if (SmoothAndProjectBone(pelvis, vPelvis) && vNeck.x != 0) {
				DrawLine(vNeck.x, vNeck.y, vPelvis.x, vPelvis.y, &BonesColor, 1.5f);
			}
			
			// Шея - плечи
			if (SmoothAndProjectBone(l_upperarm, vLShoulder) && vNeck.x != 0) {
				DrawLine(vNeck.x, vNeck.y, vLShoulder.x, vLShoulder.y, &BonesColor, 1.5f);
			}
			if (SmoothAndProjectBone(r_upperarm, vRShoulder) && vNeck.x != 0) {
				DrawLine(vNeck.x, vNeck.y, vRShoulder.x, vRShoulder.y, &BonesColor, 1.5f);
			}
			
			// Плечи - локти
			if (SmoothAndProjectBone(l_forearm, vLElbow) && vLShoulder.x != 0) {
				DrawLine(vLShoulder.x, vLShoulder.y, vLElbow.x, vLElbow.y, &BonesColor, 1.5f);
			}
			if (SmoothAndProjectBone(r_forearm, vRElbow) && vRShoulder.x != 0) {
				DrawLine(vRShoulder.x, vRShoulder.y, vRElbow.x, vRElbow.y, &BonesColor, 1.5f);
			}
			
			// Локти - руки
			if (SmoothAndProjectBone(l_hand, vLHand) && vLElbow.x != 0) {
				DrawLine(vLElbow.x, vLElbow.y, vLHand.x, vLHand.y, &BonesColor, 1.5f);
			}
			if (SmoothAndProjectBone(r_hand, vRHand) && vRElbow.x != 0) {
				DrawLine(vRElbow.x, vRElbow.y, vRHand.x, vRHand.y, &BonesColor, 1.5f);
			}
			
			// Таз - колени
			if (SmoothAndProjectBone(l_knee, vLKnee) && vPelvis.x != 0) {
				DrawLine(vPelvis.x, vPelvis.y, vLKnee.x, vLKnee.y, &BonesColor, 1.5f);
			}
			if (SmoothAndProjectBone(r_knee, vRKnee) && vPelvis.x != 0) {
				DrawLine(vPelvis.x, vPelvis.y, vRKnee.x, vRKnee.y, &BonesColor, 1.5f);
			}
			
			// Колени - ступни
			if (SmoothAndProjectBone(l_foot, vLFoot) && vLKnee.x != 0) {
				DrawLine(vLKnee.x, vLKnee.y, vLFoot.x, vLFoot.y, &BonesColor, 1.5f);
			}
			if (SmoothAndProjectBone(r_foot, vRFoot) && vRKnee.x != 0) {
				DrawLine(vRKnee.x, vRKnee.y, vRFoot.x, vRFoot.y, &BonesColor, 1.5f);
			}
		}
		
		Vector2 tempFeetR, tempFeetL;
		if (LocalPlayer.WorldToScreen(BP->GetBoneByID2(r_foot), tempFeetR) && LocalPlayer.WorldToScreen(BP->GetBoneByID2(penis), Penis) && LocalPlayer.WorldToScreen(BP->GetBoneByID2(l_foot), tempFeetL)) {
			Vector2 tempHead;
			if (LocalPlayer.WorldToScreen(BP->GetBoneByID2(jaw) + Vector3(0.f, 0.16f, 0.f), tempHead))
			{
				Vector2 tempFeet = (tempFeetR + tempFeetL) / 2.f;
				float Entity_h = tempHead.y - tempFeet.y;
				float w = Entity_h / 4;
				float Entity_x = tempFeet.x - w;
				float Entity_y = tempFeet.y;
				float Entity_w = Entity_h / 2;
				bool PlayerWounded = BP->HasFlags(64);
				bool PlayerSleeping = BP->HasFlags(16);

				if (PlayerSleeping && Value::bools::Visuals::ESP::IgnoreSleeper)
					return;
				LEFT = { 0, 0 },
					RIGHT = { 0, 0 },
					TOP = { 0, 0 },
					DOWN = { 0, 0 },
					LEFT_TOP = { 0, 0 },
					RIGHT_TOP = { 0, 0 },
					LEFT_DOWN = { 0, 0 },
					RIGHT_DOWN = { 0, 0 };
				RGBA White = { 255, 0,255,255 };
				RGBA Bot = { 0, 0,0,255 };
				//D2D1::ColorF::Enum PlayerClr = PlayerSleeping ? D2D1::ColorF::BlueViolet : PlayerWounded ? D2D1::ColorF::DarkOrange : D2D1::ColorF::Gold;
				//if (Value::bools::Visuals::ESP::Player)	DrawLine(Vars::Other::Width / 2, Vars::Other::Height, Penis.x, Penis.y, 255, 0, 255, 255);
				if (Value::bools::Visuals::ESP::Box && !PlayerSleeping && !PlayerWounded) {
					RGBA Color = { Value::Colors::Visuals::ESP::Box[0] * 255, Value::Colors::Visuals::ESP::Box[1] * 255 , Value::Colors::Visuals::ESP::Box[2] * 255, Value::Colors::Visuals::ESP::Box[3] * 255 };
					DrawRect(Entity_x, Entity_y, Entity_w, Entity_h, &Color, 2);
				}
				if (Value::bools::Visuals::ESP::Name) {

					Vector2 Coordinate = { 0, 0 };
					auto Text = BP->GetNamecChars();
					switch (NAME)
					{
					case LEFT_S: {
						Coordinate = { (Entity_x + Entity_w) - 10 - ImGui::CalcTextSize(Text.c_str()).x, (Entity_y + (Entity_h / 2)) };
						LEFT += { 0, 15 };
					}
							   break;
					case RIGHT_S: {
						Coordinate = { (Entity_x), (Entity_y + (Entity_h / 2) + 10) };
						RIGHT += { 0, 15 };
					}
								break;
					case TOP_S: {
						Coordinate = { (Entity_x + (Entity_w / 2) - (ImGui::CalcTextSize(Text.c_str()).x / 2)) , (Entity_y + Entity_h - ImGui::CalcTextSize(Text.c_str()).y - 5) };
						TOP -= { 0, 15 };
					}
							  break;
					case DOWN_S: {
						Coordinate = { (Entity_x + (Entity_w / 2) - (ImGui::CalcTextSize(Text.c_str()).x / 2)) , (Entity_y + 5) };
						DOWN += { 0, 15 };
					}
							   break;
					case LEFT_TOP_S: {
						Coordinate = { (Entity_x + Entity_w) - 10 - ImGui::CalcTextSize(Text.c_str()).x, (Entity_y + Entity_h) };
						LEFT_TOP += { 0, 15 };
					}
								   break;
					case RIGHT_TOP_S: {
						Coordinate = { Entity_x + 10, (Entity_y + Entity_h) };
						RIGHT_TOP += { 0, 15 };
					}
									break;
					case LEFT_DOWN_S: {
						Coordinate = { (Entity_x + Entity_w) - 10 - ImGui::CalcTextSize(Text.c_str()).x, Entity_y - ImGui::CalcTextSize(Text.c_str()).y };
						LEFT_DOWN -= { 0, 15 };
					}
									break;
					case RIGHT_DOWN_S: {
						Coordinate = { Entity_x + 10, Entity_y - ImGui::CalcTextSize(Text.c_str()).y };
						RIGHT_DOWN -= { 0, 15 };
					}
									 break;
					default: Coordinate = Vector2{ 0, 0 };
						   break;
					}

					RGBA Color = { Value::Colors::Visuals::ESP::Name[0] * 255, Value::Colors::Visuals::ESP::Name[1] * 255 , Value::Colors::Visuals::ESP::Name[2] * 255, Value::Colors::Visuals::ESP::Name[3] * 255 };
					DrawStrokeText(Coordinate.x, Coordinate.y, &Color, Text.c_str());
				}
				if (Value::bools::Visuals::ESP::Weapon) {
					Vector2 Coordinate = { 0, 0 };
					auto Text = BP->GetActiveWeaponcChars();
					switch (WEAPON)
					{
					case LEFT_S: {
						Coordinate = LEFT + Vector2{ (Entity_x + Entity_w) - 10 - ImGui::CalcTextSize(Text.c_str()).x, (Entity_y + (Entity_h / 2)) };
						LEFT += { 0, 15 };
					}
							   break;
					case RIGHT_S: {
						Coordinate = RIGHT + Vector2{ (Entity_x), (Entity_y + (Entity_h / 2) + 10) };
						RIGHT += { 0, 15 };
					}
								break;
					case TOP_S: {
						Coordinate = TOP + Vector2{ (Entity_x + (Entity_w / 2) - (ImGui::CalcTextSize(Text.c_str()).x / 2)) , (Entity_y + Entity_h - ImGui::CalcTextSize(Text.c_str()).y - 5) };
						TOP -= { 0, 15 };
					}
							  break;
					case DOWN_S: {
						Coordinate = DOWN + Vector2{ (Entity_x + (Entity_w / 2) - (ImGui::CalcTextSize(Text.c_str()).x / 2)) , (Entity_y + 5) };
						DOWN += { 0, 15 };
					}
							   break;
					case LEFT_TOP_S: {
						Coordinate = LEFT_TOP + Vector2{ (Entity_x + Entity_w) - 10 - ImGui::CalcTextSize(Text.c_str()).x, (Entity_y + Entity_h) };
						LEFT_TOP += { 0, 15 };
					}
								   break;
					case RIGHT_TOP_S: {
						Coordinate = RIGHT_TOP + Vector2{ Entity_x + 10, (Entity_y + Entity_h) };
						RIGHT_TOP += { 0, 15 };
					}
									break;
					case LEFT_DOWN_S: {
						Coordinate = LEFT_DOWN + Vector2{ (Entity_x + Entity_w) - 10 - ImGui::CalcTextSize(Text.c_str()).x, Entity_y - ImGui::CalcTextSize(Text.c_str()).y };
						LEFT_DOWN -= { 0, 15 };
					}
									break;
					case RIGHT_DOWN_S: {
						Coordinate = RIGHT_DOWN + Vector2{ Entity_x + 10, Entity_y - ImGui::CalcTextSize(Text.c_str()).y };
						RIGHT_DOWN -= { 0, 15 };
					}
									 break;
					default: Coordinate = Vector2{ 0, 0 };
						   break;
					}

					RGBA Color = { Value::Colors::Visuals::ESP::Weapon[0] * 255, Value::Colors::Visuals::ESP::Weapon[1] * 255 , Value::Colors::Visuals::ESP::Weapon[2] * 255, Value::Colors::Visuals::ESP::Weapon[3] * 255 };
					DrawStrokeText(Coordinate.x, Coordinate.y, &Color, Text.c_str());
				}
	if (Value::bools::Visuals::ESP::Health) {
					RGBA Color = { Value::Colors::Visuals::ESP::Health[0] * 255, Value::Colors::Visuals::ESP::Health[1] * 255 , Value::Colors::Visuals::ESP::Health[2] * 255, Value::Colors::Visuals::ESP::Health[3] * 255 };
					char Text[64];
					Vector2 Coordinate = { 0, 0 };
					sprintf(Text, "%0.f HP", BP->GetHealth());
					switch (HEALTH)
					{
					case LEFT_S: {
						Coordinate = LEFT + Vector2{ (Entity_x + Entity_w) - 10 - ImGui::CalcTextSize(Text).x, (Entity_y + (Entity_h / 2)) };
						LEFT += { 0, 15 };
					}
							   break;
					case RIGHT_S: {
						Coordinate = RIGHT + Vector2{ (Entity_x), (Entity_y + (Entity_h / 2) + 10) };
						RIGHT += { 0, 15 };
					}
								break;
					case TOP_S: {
						Coordinate = TOP + Vector2{ (Entity_x + (Entity_w / 2) - (ImGui::CalcTextSize(Text).x / 2)) , (Entity_y + Entity_h - ImGui::CalcTextSize(Text).y - 5) };
						TOP -= { 0, 15 };
					}
							  break;
					case DOWN_S: {
						Coordinate = DOWN + Vector2{ (Entity_x + (Entity_w / 2) - (ImGui::CalcTextSize(Text).x / 2)) , (Entity_y + 5) };
						DOWN += { 0, 15 };
					}
							   break;
					case LEFT_TOP_S: {
						Coordinate = LEFT_TOP + Vector2{ (Entity_x + Entity_w) - 10 - ImGui::CalcTextSize(Text).x, (Entity_y + Entity_h) };
						LEFT_TOP += { 0, 15 };
					}
								   break;
					case RIGHT_TOP_S: {
						Coordinate = RIGHT_TOP + Vector2{ Entity_x + 10, (Entity_y + Entity_h) };
						RIGHT_TOP += { 0, 15 };
					}
									break;
					case LEFT_DOWN_S: {
						Coordinate = LEFT_DOWN + Vector2{ (Entity_x + Entity_w) - 10 - ImGui::CalcTextSize(Text).x, Entity_y - ImGui::CalcTextSize(Text).y };
						LEFT_DOWN -= { 0, 15 };
					}
									break;
					case RIGHT_DOWN_S: {
						Coordinate = RIGHT_DOWN + Vector2{ Entity_x + 10, Entity_y - ImGui::CalcTextSize(Text).y };
						RIGHT_DOWN -= { 0, 15 };
					}
									 break;
					default: Coordinate = Vector2{ 0, 0 };
						   break;
					}
					DrawStrokeText(Coordinate.x, Coordinate.y, &Color, Text);
				}
				if (Value::bools::Visuals::ESP::Distance) {
					Vector2 Coordinate = { 0, 0 };
					char Text[64];
					sprintf(Text, "%d M", (int)Math::Calc3D_Dist(LP->GetBoneByID2(head), BP->GetBoneByID2(head)));
					switch (DISTANCE)
					{
					case LEFT_S: {
						Coordinate = LEFT + Vector2{ (Entity_x + Entity_w) - 10 - ImGui::CalcTextSize(Text).x, (Entity_y + (Entity_h / 2)) };
						LEFT += { 0, 15 };
					}
							   break;
					case RIGHT_S: {
						Coordinate = RIGHT + Vector2{ (Entity_x), (Entity_y + (Entity_h / 2) + 10) };
						RIGHT += { 0, 15 };
					}
								break;
					case TOP_S: {
						Coordinate = TOP + Vector2{ (Entity_x + (Entity_w / 2) - (ImGui::CalcTextSize(Text).x / 2)) , (Entity_y + Entity_h - ImGui::CalcTextSize(Text).y - 5) };
						TOP -= { 0, 15 };
					}
							  break;
					case DOWN_S: {
						Coordinate = DOWN + Vector2{ (Entity_x + (Entity_w / 2) - (ImGui::CalcTextSize(Text).x / 2)) , (Entity_y + 5) };
						DOWN += { 0, 15 };
					}
							   break;
					case LEFT_TOP_S: {
						Coordinate = LEFT_TOP + Vector2{ (Entity_x + Entity_w) - 10 - ImGui::CalcTextSize(Text).x, (Entity_y + Entity_h) };
						LEFT_TOP += { 0, 15 };
					}
								   break;
					case RIGHT_TOP_S: {
						Coordinate = RIGHT_TOP + Vector2{ Entity_x + 10, (Entity_y + Entity_h) };
						RIGHT_TOP += { 0, 15 };
					}
									break;
					case LEFT_DOWN_S: {
						Coordinate = LEFT_DOWN + Vector2{ (Entity_x + Entity_w) - 10 - ImGui::CalcTextSize(Text).x, Entity_y - ImGui::CalcTextSize(Text).y };
						LEFT_DOWN -= { 0, 15 };
					}
									break;
					case RIGHT_DOWN_S: {
						Coordinate = RIGHT_DOWN + Vector2{ Entity_x + 10, Entity_y - ImGui::CalcTextSize(Text).y };
						RIGHT_DOWN -= { 0, 15 };
					}
									 break;
					default: Coordinate = Vector2{ 0, 0 };
						   break;
					}

					RGBA Color = { Value::Colors::Visuals::ESP::Distance[0] * 255, Value::Colors::Visuals::ESP::Distance[1] * 255 , Value::Colors::Visuals::ESP::Distance[2] * 255, Value::Colors::Visuals::ESP::Distance[3] * 255 };

					DrawStrokeText(Coordinate.x, Coordinate.y, &Color, Text);
				}
			}
		}
	}
}
// Функция для очистки кэша неактивных игроков
void CleanupESPSmoothCache() {
	UINT64 currentTime = GetTickCount64();
	std::vector<UINT64> keysToRemove;

	for (auto& pair : g_ESPSmoothCache) {
		// Если данные не обновлялись более 10 секунд, удаляем их
		if (currentTime - pair.second.lastUpdateTime > 10000) {
			keysToRemove.push_back(pair.first);
		}
	}

	for (auto key : keysToRemove) {
		g_ESPSmoothCache.erase(key);
	}
}

float W = 200.f, H = 140;

void PlayerInfoPlayer(BasePlayer* BP) {
	if (Value::bools::Visuals::PlayerPanel::Enable) {
		float Pos = 0;
		ImGui::SetNextWindowSize({ W, H });
		ImGui::Begin("PlayerInfo", NULL, ImGuiWindowFlags_NoDecoration);
		auto pos = ImGui::GetWindowPos();
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		window->DrawList->AddRectFilled({ pos.x - 10, pos.y - 10 }, { pos.x + W, pos.y + H }, ImGui::ColorConvertFloat4ToU32({ 0.25 , 0.25 , 0.25 , 1 }), 2);
		RGBA col = { 192, 192, 192, 255 };
		if (Value::bools::Visuals::PlayerPanel::Name) {
			auto nickname = BP->GetNamecChars();
			window->DrawList->AddText({ pos.x + (W / 2) - (ImGui::CalcTextSize(nickname.c_str()).x / 2), pos.y + 5 }, ImGui::ColorConvertFloat4ToU32({ 0.75 , 0.75 , 0.75 , 1 }), nickname.c_str());
		}
		window->DrawList->AddLine({ pos.x, pos.y + 23 }, { pos.x + W,  pos.y + 23 }, ImGui::ColorConvertFloat4ToU32({ 0.75 , 0.75 , 0.75 , 1 }), 1);
		if (Value::bools::Visuals::PlayerPanel::Weapons)
			for (int i = 0; i < 6; i++)
			{
				BaseProjectile* GetWeaponInfo = BP->GetWeaponInfo(i);
				std::cout <<"huesos negro " << GetWeaponInfo << std::endl;
				if (GetWeaponInfo)
				{
					auto Item = BP->GetWeaponInfo(i)->GetName();
					auto name = readchar(Item);
					if (name.length() < 20)
					{
						auto text = name.c_str();
						ImVec2 textsize = ImGui::CalcTextSize(text);
						window->DrawList->AddText({ pos.x + (W / 2) - textsize.x / 2, pos.y + 25 + Pos }, ImGui::ColorConvertFloat4ToU32({ 0.75 , 0.75 , 0.75 , 1 }), text);
						//DrawStrokeText( pos.x + (W / 2) - textsize.x / 2, pos.y + 40 + Pos , &col , text);
					}
				}
				else
				{
					window->DrawList->AddText({ pos.x + (W / 2) - ImGui::CalcTextSize("Noting").x / 2, pos.y + 20 + Pos }, ImGui::ColorConvertFloat4ToU32({ 0.75 , 0.75 , 0.75 , 1 }), "Nothing");
					//DrawStrokeText( pos.x + (W / 2) - ImGui::CalcTextSize("Noting").x / 2, pos.y + 40 + Pos , &col, "Nothing");
				}
				Pos += 15;
			}
		if (Value::bools::Visuals::PlayerPanel::HP) {
			float health = BP->GetHealth();
			float maxheal = 100.f;
			window->DrawList->AddRect({ pos.x + (W / 2) - 50, pos.y + 27 + Pos }, { pos.x + (W / 2) - 50 + 100, pos.y + 27 + Pos + 15 }, ImGui::ColorConvertFloat4ToU32({ 0.75 , 0.75 , 0.75 , 1 }));
			window->DrawList->AddRectFilled({ pos.x + (W / 2) + -49, pos.y + 28 + Pos }, { pos.x + (W / 2) + -49 + (98 * (health / maxheal)), pos.y + 28 + Pos + 13 }, ImGui::ColorConvertFloat4ToU32({ 0.75 , 0.75 , 0.75 , 1 }), 2);
		}
		ImGui::End();
	}
}


float FOV = 1000000.f;
int EntityCount;
int RenderedEntityCount;
int RenderedPlayerCount;
RGBA Purple = { 255, 0, 255, 128 };
RGBA Gray = { 128, 128, 128, 128 };
void entity_esp_thread() {

	// Добавляем структуры и переменные для сглаживания в начало функции
	static bool initialized = false;

	if (!initialized) {
		// Инициализация структур только один раз
		g_ESPSmoothCache.clear();
		initialized = true;
	}

	char ec[64];
	sprintf(ec, "Entity : %i", EntityCount);
	DrawStrokeText((Value::floats::Screen::W / 2) - (ImGui::CalcTextSize(ec).x / 2), Value::floats::Screen::H - 30, &Grey, ec);
	char rpc[64];
	sprintf(rpc, "Player Count : %i", RenderedEntityCount);
	DrawStrokeText((Value::floats::Screen::W / 2) - (ImGui::CalcTextSize(rpc).x / 2), Value::floats::Screen::H - 45, &Grey, rpc);
	char rec[64];
	sprintf(rec, "Rendered Entity Count : %i", RenderedEntityCount);
	DrawStrokeText((Value::floats::Screen::W / 2) - (ImGui::CalcTextSize(rec).x / 2), Value::floats::Screen::H - 60, &Grey, rec);

	DrawFilledRect(Width - 300 - 25, 20 + 25, 300, 300, &Gray); //������ ��� - �����
	DrawFilledRect(Width - 300 - 25, 0 + 25, 300, 20, &Purple); //Title bar - ����������

	DrawLine(Width - 150 - 25, 0 + 25 + 20, Width - 150 - 25, 0 + 25 + 20 + 300, &Purple, 1); //�������#1 - ����������
	DrawLine(Width - 300 - 25, 0 + 25 + 20 + 150, Width - 25, 0 + 25 + 20 + 150, &Purple, 1); //�������#2 - ����������

	DrawRect(Width - 300 - 25, 0 + 25, 300, 320, &Purple, 1); //���� - ����������

	if (!(AimEntity->IsDead()) && Value::bools::Aim::TargetLine) {
		Vector2 ScreenPos;
		RGBA Color = { Value::Colors::Aim::TargetLine[0] * 255, Value::Colors::Aim::TargetLine[1] * 255 , Value::Colors::Aim::TargetLine[2] * 255, Value::Colors::Aim::TargetLine[3] * 255 };
		if (LocalPlayer.WorldToScreen(AimEntity->GetBoneByID2(spine1), ScreenPos))
			DrawLine(Value::floats::Screen::W / 2, Value::floats::Screen::H, ScreenPos.x, ScreenPos.y, &Color, 2);
	}
	if (!(AimEntity->IsDead()) && Value::bools::Visuals::PlayerPanel::Enable) {
		PlayerInfoPlayer(AimEntity);
	}

	RenderedEntityCount = NULL; RenderedPlayerCount = NULL;

	{
		if (Value::bools::Visuals::World::Items::Stash) {
			for (Vector3 Pos : Stash) {
				Vector2 pos;
				if (LocalPlayer.WorldToScreen(Pos, pos)) {
					int distation = (int)Math::Calc3D_Dist(LocalPlayer.BasePlayer->GetBoneByID2(head), Pos);
					char text[0x100];
					sprintf(text, "Stash [%d m]", distation);
					if (distation <= Value::floats::Visuals::World::LimitDistance) {
						RenderedEntityCount++;
						RGBA Color = { Value::Colors::Visuals::World::Stash[0] * 255, Value::Colors::Visuals::World::Stash[1] * 255 , Value::Colors::Visuals::World::Stash[2] * 255, Value::Colors::Visuals::World::Stash[3] * 255 };
						DrawStrokeText(pos.x, pos.y, &Color, text);
					}
				}
			}
		}
		if (Value::bools::Visuals::World::Items::Hemp) {
			for (Vector3 Pos : hemp) {
				Vector2 pos;
				if (LocalPlayer.WorldToScreen(Pos, pos)) {
					int distation = (int)Math::Calc3D_Dist(LocalPlayer.BasePlayer->GetBoneByID2(head), Pos);
					char text[0x100];
					sprintf(text, "Hemp [%d m]", distation);
					if (distation <= Value::floats::Visuals::World::LimitDistance) {
						RenderedEntityCount++;
						RGBA Color = { Value::Colors::Visuals::World::Hemp[0] * 255, Value::Colors::Visuals::World::Hemp[1] * 255 , Value::Colors::Visuals::World::Hemp[2] * 255, Value::Colors::Visuals::World::Hemp[3] * 255 };
						DrawStrokeText(pos.x, pos.y, &Color, text);
					}
				}
			}
		}
		if (Value::bools::Visuals::World::Items::AirDrop) {
			for (Vector3 Pos : Airdrop) {
				Vector2 pos;
				if (LocalPlayer.WorldToScreen(Pos, pos)) {
					int distation = (int)Math::Calc3D_Dist(LocalPlayer.BasePlayer->GetBoneByID2(head), Pos);
					char text[0x100];
					sprintf(text, "AirDrop [%d m]", distation);
					if (distation <= Value::floats::Visuals::World::LimitDistance2) {
						RenderedEntityCount++;
						RGBA Color = { Value::Colors::Visuals::World::AirDrop[0] * 255, Value::Colors::Visuals::World::AirDrop[1] * 255 , Value::Colors::Visuals::World::AirDrop[2] * 255, Value::Colors::Visuals::World::AirDrop[3] * 255 };
						DrawStrokeText(pos.x, pos.y, &Color, text);
					}
				}
			}
		}
		if (Value::bools::Visuals::World::Items::CH47) {
			for (Vector3 Pos : hackable_crate) {
				Vector2 pos;
				if (LocalPlayer.WorldToScreen(Pos, pos)) {
					int distation = (int)Math::Calc3D_Dist(LocalPlayer.BasePlayer->GetBoneByID2(head), Pos);
					char text[0x100];
					sprintf(text, "CH47 [%d m]", distation);
					if (distation <= Value::floats::Visuals::World::LimitDistance2) {
						RenderedEntityCount++;
						RGBA Color = { Value::Colors::Visuals::World::CH47[0] * 255, Value::Colors::Visuals::World::CH47[1] * 255 , Value::Colors::Visuals::World::CH47[2] * 255, Value::Colors::Visuals::World::CH47[3] * 255 };
						DrawStrokeText(pos.x, pos.y, &Color, text);
					}
				}
			}
		}
		if (Value::bools::Visuals::World::Items::Minicopter) {
			for (Vector3 Pos : vehicles) {
				Vector2 pos;
				if (LocalPlayer.WorldToScreen(Pos, pos)) {
					int distation = (int)Math::Calc3D_Dist(LocalPlayer.BasePlayer->GetBoneByID2(head), Pos);
					char text[0x100];
					if (distation <= Value::floats::Visuals::World::LimitDistance2) {
						RenderedEntityCount++;
						sprintf(text, "Minicopter [%d m]", distation);
						RGBA Color = { Value::Colors::Visuals::World::Patrol[0] * 255, Value::Colors::Visuals::World::Patrol[1] * 255 , Value::Colors::Visuals::World::Patrol[2] * 255, Value::Colors::Visuals::World::Patrol[3] * 255 };
						DrawStrokeText(pos.x, pos.y, &Color, text);
					}
				}
			}
		}
		if (Value::bools::Visuals::World::Items::Patrol) {
			for (Vector3 Pos : patrol_heli) {
				Vector2 pos;
				if (LocalPlayer.WorldToScreen(Pos, pos)) {
					int distation = (int)Math::Calc3D_Dist(LocalPlayer.BasePlayer->GetBoneByID2(head), Pos);
					char text[0x100];
					if (distation <= Value::floats::Visuals::World::LimitDistance2) {
						RenderedEntityCount++;
						sprintf(text, "Patrol [%d m]", distation);
						RGBA Color = { Value::Colors::Visuals::World::Patrol[0] * 255, Value::Colors::Visuals::World::Patrol[1] * 255 , Value::Colors::Visuals::World::Patrol[2] * 255, Value::Colors::Visuals::World::Patrol[3] * 255 };
						DrawStrokeText(pos.x, pos.y, &Color, text);
					}
				}
			}
		}
		if (Value::bools::Visuals::World::Items::Ore::Stone) {
			for (Vector3 Pos : StoneNodes) {
				Vector2 pos;
				if (LocalPlayer.WorldToScreen(Pos, pos)) {
					int distation = (int)Math::Calc3D_Dist(LocalPlayer.BasePlayer->GetBoneByID2(head), Pos);
					char text[0x100];
					sprintf(text, "Stone [%d m]", distation);
					if (distation <= Value::floats::Visuals::World::LimitDistance) {
						RenderedEntityCount++;
						RGBA Color = { Value::Colors::Visuals::World::Ore::Stone[0] * 255, Value::Colors::Visuals::World::Ore::Stone[1] * 255 , Value::Colors::Visuals::World::Ore::Stone[2] * 255, Value::Colors::Visuals::World::Ore::Stone[3] * 255 };
						DrawStrokeText(pos.x, pos.y, &Color, text);
					}
				}
			}
		}
		if (Value::bools::Visuals::World::Items::Ore::Iron) {
			for (Vector3 Pos : MetalNodes) {
				Vector2 pos;
				if (LocalPlayer.WorldToScreen(Pos, pos)) {
					int distation = (int)Math::Calc3D_Dist(LocalPlayer.BasePlayer->GetBoneByID2(head), Pos);
					char text[0x100];
					sprintf(text, "Metal [%d m]", distation);
					if (distation <= Value::floats::Visuals::World::LimitDistance) {
						RenderedEntityCount++;
						RGBA Color = { Value::Colors::Visuals::World::Ore::Iron[0] * 255, Value::Colors::Visuals::World::Ore::Iron[1] * 255 , Value::Colors::Visuals::World::Ore::Iron[2] * 255, Value::Colors::Visuals::World::Ore::Iron[3] * 255 };
						DrawStrokeText(pos.x, pos.y, &Color, text);
					}
				}
			}
		}
		if (Value::bools::Visuals::World::Items::Ore::Sulfur) {
			for (Vector3 Pos : SulfurNodes) {
				Vector2 pos;
				if (LocalPlayer.WorldToScreen(Pos, pos)) {
					int distation = (int)Math::Calc3D_Dist(LocalPlayer.BasePlayer->GetBoneByID2(head), Pos);
					char text[0x100];
					sprintf(text, "Sulfur [%d m]", distation);
					if (distation <= Value::floats::Visuals::World::LimitDistance) {
						RenderedEntityCount++;
						RGBA Color = { Value::Colors::Visuals::World::Ore::Sulfur[0] * 255, Value::Colors::Visuals::World::Ore::Sulfur[1] * 255 , Value::Colors::Visuals::World::Ore::Sulfur[2] * 255, Value::Colors::Visuals::World::Ore::Sulfur[3] * 255 };
						DrawStrokeText(pos.x, pos.y, &Color, text);
					}
				}
			}
		}

		if (Value::bools::Visuals::ESP::Corpse) {
			for (Vector3 Pos : corpse) {
				Vector2 pos;
				if (LocalPlayer.WorldToScreen(Pos, pos)) {
					int distation = (int)Math::Calc3D_Dist(LocalPlayer.BasePlayer->GetBoneByID2(head), Pos);
					char text[0x100];
					sprintf(text, "Corpse [%d m]", distation);
					if (distation <= Value::floats::Visuals::World::LimitDistance) {
						RenderedEntityCount++;
						RGBA Color = { Value::Colors::Visuals::World::Corpse[0] * 255, Value::Colors::Visuals::World::Corpse[1] * 255 , Value::Colors::Visuals::World::Corpse[2] * 255, Value::Colors::Visuals::World::Corpse[3] * 255 };
						DrawStrokeText(pos.x, pos.y, &Color, text);
					}
				}
			}
		}


		if (Value::bools::Visuals::ESP::Backpack) {
			for (Vector3 Pos : backpack) {
				Vector2 pos;
				if (LocalPlayer.WorldToScreen(Pos, pos)) {
					int distation = (int)Math::Calc3D_Dist(LocalPlayer.BasePlayer->GetBoneByID2(head), Pos);
					char text[0x100];
					sprintf(text, "Backpack [%d m]", distation);
					if (distation <= Value::floats::Visuals::World::LimitDistance) {
						RenderedEntityCount++;
						RGBA Color = { Value::Colors::Visuals::World::Backpack[0] * 255, Value::Colors::Visuals::World::Backpack[1] * 255 , Value::Colors::Visuals::World::Backpack[2] * 255, Value::Colors::Visuals::World::Backpack[3] * 255 };
						DrawStrokeText(pos.x, pos.y, &Color, text);
					}
				}
			}
		}
		if (true) {
			for (BasePlayer* Player : otherplayers) {
				if (!Player->IsValid())
					continue;

				// Заменяем двойной вызов ESP на один вызов нашей улучшенной функции
				ESP(Player, LocalPlayer.BasePlayer);
				RenderedPlayerCount++;
				RenderedEntityCount++;
				RadarPlayer(Player);
				// Удаляем повторный вызов ESP(Player, LocalPlayer.BasePlayer);

				if (Value::bools::Aim::IgnoreSleepers && Player->HasFlags(16))
					continue;
				if (Player->IsVisible() && (AimFov(Player) < Value::floats::Aim::Fov))
				{
					AimEntity = Player;
					Aim(AimEntity);
				}
			}
		}
	}

	// Добавляем периодическую очистку кэша
	static UINT64 lastCleanupTime = 0;
	UINT64 currentTime = GetTickCount64();

	if (currentTime - lastCleanupTime > 30000) { // Очистка каждые 30 секунд
		CleanupESPSmoothCache();
		lastCleanupTime = currentTime;
	}
	
}
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) {
		return true;
	}
	switch (msg)
	{
	case WM_DESTROY:
		ClearD3D();
		PostQuitMessage(0);
		exit(4);
		break;
	case WM_SIZE:
		if (pDevice != NULL && wParam != SIZE_MINIMIZED) {
			ImGui_ImplDX9_InvalidateDeviceObjects();
			pParams.BackBufferWidth = LOWORD(lParam);
			pParams.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = pDevice->Reset(&pParams);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
		break;
	}
	return 0;
}

std::vector<BasePlayer*> local_players;
std::vector<Vector3> local_Stash;
std::vector<Vector3> local_hemp;
std::vector<Vector3> local_corpse;
std::vector<Vector3> local_backpack;
std::vector<Vector3> local_vehicles;
std::vector<Vector3> local_Airdrop;
std::vector<Vector3> local_patrol_heli;
std::vector<Vector3> local_hackable_crate;
std::vector<Vector3> local_high_tier_crates;
std::vector<Vector3> local_low_tier_crates;
//std::vector<EntityG> local_tool_cupboard;
//std::vector<EntityG> local_food;
//std::vector<EntityG> local_cargo_ship;
std::vector<Vector3> local_DroppedItem;
//std::vector<EntityG> local_Animal;
std::vector<Vector3> local_SulfurNodes;
std::vector<Vector3> local_StoneNodes;
std::vector<Vector3> local_MetalNodes;
//std::vector<EntityG> local_high_tier_crates;
//std::vector<EntityG> local_low_tier_crates;

DWORD __stdcall EntityThread(LPVOID lpParameter) {
	while (true) {
		auto BaseNetworkable = safe_read(gBase + oBaseNetworkable, DWORD64);
		if (!BaseNetworkable)
			continue;
		DWORD64 EntityRealm = safe_read(BaseNetworkable + 0xB8, DWORD64);
		if (!EntityRealm)
			continue;
		DWORD64 ClientEntities = safe_read(EntityRealm, DWORD64);
		if (!ClientEntities)
			continue;
		DWORD64 ClientEntities_list = safe_read(ClientEntities + 0x10, DWORD64);
		if (!ClientEntities_list)
			continue;
		DWORD64 ClientEntities_values = safe_read(ClientEntities_list + 0x28, DWORD64);
		if (!ClientEntities_values)
			continue;
		EntityCount = safe_read(ClientEntities_values + 0x10, int);
		if (!EntityCount)
			continue;
		auto EntityBuffer = safe_read(ClientEntities_values + 0x18, uintptr_t);
		if (!EntityBuffer)
			continue;
		for (int i = 0; i <= EntityCount; i++)
		{
			DWORD64 Entity = safe_read(EntityBuffer + 0x20 + (i * 0x8), DWORD64);
			if (Entity <= 100000) continue;
			DWORD64 Object = safe_read(Entity + 0x10, DWORD64); //BaseObject
			if (Object <= 100000) continue;
			DWORD64 ObjectClass = safe_read(Object + 0x30, DWORD64); //Object
			//
			WORD tag = safe_read(ObjectClass + 0x54, WORD);
			//
			if (ObjectClass <= 100000) continue;
			uintptr_t name = safe_read(ObjectClass + 0x60, uintptr_t);
			if (!name) continue;
			auto buff = readchar(name);
			//std::cout << buff << std::endl;
			if (buff.find("Local") != std::string::npos) {
				auto Player = (BasePlayer*)safe_read(Object + 0x28, DWORD64);
				if (!safe_read((uintptr_t)Player + 0x4C0, DWORD64)) continue; // public PlayerModel playerModel;??? 0x4B0
				LocalPlayer.BasePlayer = Player;
			}

			else
				if (buff.find("player.prefab") != std::string::npos || buff.find("scientist") != std::string::npos/* && (!strstr(buff.c_str(), "prop") && !strstr(buff.c_str(), "corpse"))*/)
				{
					BasePlayer* Player = (BasePlayer*)safe_read(Object + 0x28, DWORD64); //public ulong playerID;
					//if (!safe_read((uintptr_t)Player + oPlayerModel, DWORD64)) continue;// 0x4A8 public PlayerModel playerModel;
					//if (!Player->IsValid()) continue;
					local_players.push_back(Player);
				}
				else
					if (strstr(buff.c_str(), "small_stash_deployed.prefab")) {
						auto flag = safe_read(Entity + 0x130, uintptr_t);
						if (flag != 2048)
							continue;
						DWORD64 gameObject = safe_read(ObjectClass + 0x30, DWORD64); //Tag 449
						DWORD64 Trans = safe_read(gameObject + 0x8, DWORD64);
						DWORD64 Vec = safe_read(Trans + 0x38, DWORD64);
						Vector3 pos = safe_read(Vec + 0x90, Vector3);

						local_Stash.push_back(pos);
					}
					else
						if (strstr(buff.c_str(), "hemp-collectable.prefab")) {
							DWORD64 gameObject = safe_read(ObjectClass + 0x30, DWORD64); //Tag 449
							DWORD64 Trans = safe_read(gameObject + 0x8, DWORD64);
							DWORD64 Vec = safe_read(Trans + 0x38, DWORD64);
							Vector3 pos = safe_read(Vec + 0x90, Vector3);

							local_hemp.push_back(pos);
						}
						else
							if (strstr(buff.c_str(), "supply_drop.prefab")) {
								DWORD64 gameObject = safe_read(ObjectClass + 0x30, DWORD64); //Tag 449
								DWORD64 Trans = safe_read(gameObject + 0x8, DWORD64);
								DWORD64 Vec = safe_read(Trans + 0x38, DWORD64);
								Vector3 pos = safe_read(Vec + 0x90, Vector3);

								local_Airdrop.push_back(pos);
							}
							else
								if (strstr(buff.c_str(), "codelockedhackablecrate.prefab")) {
									DWORD64 gameObject = safe_read(ObjectClass + 0x30, DWORD64); //Tag 449
									DWORD64 Trans = safe_read(gameObject + 0x8, DWORD64);
									DWORD64 Vec = safe_read(Trans + 0x38, DWORD64);
									Vector3 pos = safe_read(Vec + 0x90, Vector3);

									local_hackable_crate.push_back(pos);
								}
								else
									if (strstr(buff.c_str(), "player_corpse.prefab")) {
										DWORD64 gameObject = safe_read(ObjectClass + 0x30, DWORD64); //Tag 449
										DWORD64 Trans = safe_read(gameObject + 0x8, DWORD64);
										DWORD64 Vec = safe_read(Trans + 0x38, DWORD64);
										Vector3 pos = safe_read(Vec + 0x90, Vector3);

										local_corpse.push_back(pos);
									}
									else
										if (strstr(buff.c_str(), "item_drop_backpack.prefab")) {
											DWORD64 gameObject = safe_read(ObjectClass + 0x30, DWORD64); //Tag 449
											DWORD64 Trans = safe_read(gameObject + 0x8, DWORD64);
											DWORD64 Vec = safe_read(Trans + 0x38, DWORD64);
											Vector3 pos = safe_read(Vec + 0x90, Vector3);

											local_backpack.push_back(pos);
										}
										else
											if (strstr(buff.c_str(), "minicopter.entity.prefab")) {
												DWORD64 gameObject = safe_read(ObjectClass + 0x30, DWORD64); //Tag 449
												DWORD64 Trans = safe_read(gameObject + 0x8, DWORD64);
												DWORD64 Vec = safe_read(Trans + 0x38, DWORD64);
												Vector3 pos = safe_read(Vec + 0x90, Vector3);

												local_vehicles.push_back(pos);
											}
											else
												if (strstr(buff.c_str(), "patrolhelicopter.prefab")) {
													DWORD64 gameObject = safe_read(ObjectClass + 0x30, DWORD64); //Tag 449
													DWORD64 Trans = safe_read(gameObject + 0x8, DWORD64);
													DWORD64 Vec = safe_read(Trans + 0x38, DWORD64);
													Vector3 pos = safe_read(Vec + 0x90, Vector3);

													local_patrol_heli.push_back(pos);
												}
												else
													if (strstr(buff.c_str(), "stone-ore.prefab")) {
														DWORD64 gameObject = safe_read(ObjectClass + 0x30, DWORD64); //Tag 449
														DWORD64 Trans = safe_read(gameObject + 0x8, DWORD64);
														DWORD64 Vec = safe_read(Trans + 0x38, DWORD64);
														Vector3 pos = safe_read(Vec + 0x90, Vector3);

														local_StoneNodes.push_back(pos);
													}
													else
														if (strstr(buff.c_str(), "metal-ore.prefab")) {
															DWORD64 gameObject = safe_read(ObjectClass + 0x30, DWORD64); //Tag 449
															DWORD64 Trans = safe_read(gameObject + 0x8, DWORD64);
															DWORD64 Vec = safe_read(Trans + 0x38, DWORD64);
															Vector3 pos = safe_read(Vec + 0x90, Vector3);

															local_MetalNodes.push_back(pos);
														}
														else
															if (strstr(buff.c_str(), "sulfur-ore.prefab")) {
																DWORD64 gameObject = safe_read(ObjectClass + 0x30, DWORD64); //Tag 449
																DWORD64 Trans = safe_read(gameObject + 0x8, DWORD64);
																DWORD64 Vec = safe_read(Trans + 0x38, DWORD64);
																Vector3 pos = safe_read(Vec + 0x90, Vector3);

																local_SulfurNodes.push_back(pos);
															}
		}

		InitLocalPlayer();
		std::this_thread::sleep_for(std::chrono::milliseconds(5000));
		std::lock_guard<std::mutex>lk(entity_mutex);
		otherplayers = std::move(local_players);
		Stash = std::move(local_Stash);
		corpse = std::move(local_corpse);
		vehicles = std::move(local_vehicles);
		DroppedItem = std::move(local_DroppedItem);
		//Animal = std::move(local_Animal);
		SulfurNodes = std::move(local_SulfurNodes);
		StoneNodes = std::move(local_StoneNodes);
		MetalNodes = std::move(local_MetalNodes);

		Airdrop = std::move(local_Airdrop);
		patrol_heli = std::move(local_patrol_heli);
		//tool_cupboard = std::move(local_tool_cupboard);
		hackable_crate = std::move(local_hackable_crate);
		hemp = std::move(local_hemp);
		backpack = std::move(local_backpack);

		//food = std::move(local_food);
		//cargo_ship = std::move(local_cargo_ship);
		high_tier_crates = std::move(local_high_tier_crates);
		low_tier_crates = std::move(local_low_tier_crates);


	}
}


BasePlayer* GetLocalPlayer() {
	return safe_read(safe_read(safe_read(gBase + 51474120, uintptr_t) + 0xB8, uintptr_t), BasePlayer*);
}

void Loop()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{
		if (PeekMessage(&Message, Window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}

		HWND hwnd_active = GetForegroundWindow();

		if (hwnd_active == hwnd) {
			HWND hwndtest = GetWindow(hwnd_active, GW_HWNDPREV);
			SetWindowPos(Window, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}

		//if (GetAsyncKeyState(0x23) & 1)
		//	exit(8);

		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(hwnd, &rc);
		ClientToScreen(hwnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = hwnd;
		io.DeltaTime = 1.0f / 100.0f;

		POINT p;
		GetCursorPos(&p);
		io.MousePos.x = (float)p.x - (float)xy.x;
		io.MousePos.y = (float)p.y - (float)xy.y;

		if (GetAsyncKeyState(VK_LBUTTON)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;

		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;

			Width = rc.right;
			Height = rc.bottom;
			wLeft = rc.left;
			wTop = rc.top;

			d3dpp.BackBufferWidth = Width;
			d3dpp.BackBufferHeight = Height;
			SetWindowPos(Window, (HWND)0, xy.x, xy.y, Width, Height, SWP_NOREDRAW);
			D3dDevice->Reset(&d3dpp);
		}
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
		ImGui::PushFont(m_pFont2);
		if (GetAsyncKeyState(VK_F2) & 1) {
			ShowMenu = !ShowMenu;
			Sleep(1);
		}

		if (ShowMenu)
		{
			// Настраиваем стиль ImGui в темных тонах
			ImGui::StyleColorsDark();
			ImVec4* colors = ImGui::GetStyle().Colors;
			colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 0.95f);
			colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.26f, 0.54f);
			colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
			colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.24f, 0.25f, 1.00f);
			colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.33f, 1.00f);
			colors[ImGuiCol_TitleBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
			colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
			colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.16f, 0.16f, 0.18f, 0.75f);
			colors[ImGuiCol_MenuBarBg] = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
			colors[ImGuiCol_CheckMark] = ImVec4(0.84f, 0.37f, 0.96f, 1.00f);
			colors[ImGuiCol_SliderGrab] = ImVec4(0.84f, 0.37f, 0.96f, 1.00f);
			colors[ImGuiCol_SliderGrabActive] = ImVec4(0.98f, 0.39f, 1.00f, 1.00f);
			colors[ImGuiCol_Button] = ImVec4(0.20f, 0.20f, 0.22f, 1.00f);
			colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.28f, 0.30f, 1.00f);
			colors[ImGuiCol_ButtonActive] = ImVec4(0.33f, 0.33f, 0.35f, 1.00f);
			colors[ImGuiCol_Header] = ImVec4(0.25f, 0.25f, 0.27f, 1.00f);
			colors[ImGuiCol_HeaderHovered] = ImVec4(0.33f, 0.33f, 0.35f, 1.00f);
			colors[ImGuiCol_HeaderActive] = ImVec4(0.40f, 0.40f, 0.44f, 1.00f);
			colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
			colors[ImGuiCol_TabHovered] = ImVec4(0.84f, 0.37f, 0.96f, 0.80f);
			colors[ImGuiCol_TabActive] = ImVec4(0.84f, 0.37f, 0.96f, 1.00f);
			colors[ImGuiCol_TabUnfocused] = ImVec4(0.18f, 0.18f, 0.20f, 1.00f);
			colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.36f, 0.36f, 0.38f, 1.00f);

			// Настраиваем размеры и закругления
			ImGuiStyle& style = ImGui::GetStyle();
			style.WindowRounding = 4.0f;
			style.FrameRounding = 4.0f;
			style.GrabRounding = 3.0f;
			style.TabRounding = 3.0f;
			style.ScrollbarRounding = 3.0f;
			style.FramePadding = ImVec2(8, 6);
			style.ItemSpacing = ImVec2(8, 6);
			style.ItemInnerSpacing = ImVec2(4, 4);

			// Создаем окно меню
			ImGui::SetNextWindowSize(ImVec2(650, 450), ImGuiCond_FirstUseEver);
			ImGui::Begin("Fine EXTERNAL", NULL, ImGuiWindowFlags_NoCollapse);

			// Определяем вкладки
			static int currentTab = 0;
			ImGui::BeginTabBar("MainTabBar", ImGuiTabBarFlags_None);

			// Вкладка ESP
			if (ImGui::BeginTabItem("ESP"))
			{
				ImGui::BeginChild("ESP Settings", ImVec2(0, 0), true);
				ImGui::Checkbox("Enable##ESP", &Value::bools::Visuals::ESP::Enable);
				
				if (Value::bools::Visuals::ESP::Enable) {
					ImGui::Separator();
					
					// Первый столбец
					ImGui::BeginGroup();
					ImGui::Checkbox("Box", &Value::bools::Visuals::ESP::Box);
					ImGui::SameLine();
					ImGui::ColorEdit4("##Box", Value::Colors::Visuals::ESP::Box, ImGuiColorEditFlags_NoInputs);
					
					ImGui::Checkbox("Name", &Value::bools::Visuals::ESP::Name);
					ImGui::SameLine();
					ImGui::ColorEdit4("##Name", Value::Colors::Visuals::ESP::Name, ImGuiColorEditFlags_NoInputs);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(100);
					ImGui::Combo("##NAME", &NAME, Items, IM_ARRAYSIZE(Items));

					ImGui::Checkbox("Health", &Value::bools::Visuals::ESP::Health);
					ImGui::SameLine();
					ImGui::ColorEdit4("##Health", Value::Colors::Visuals::ESP::Health, ImGuiColorEditFlags_NoInputs);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(100);
					ImGui::Combo("##HEALTH", &HEALTH, Items, IM_ARRAYSIZE(Items));
					
					// Добавляем новую опцию для уменьшения мерцания костей
					ImGui::Checkbox("Reduce Bone Flicker", &Value::bools::Visuals::ESP::ReduceBoneFlicker);
					ImGui::SameLine();
					ImGui::TextDisabled("(?)");
					if (ImGui::IsItemHovered())
						ImGui::SetTooltip("Уменьшает мерцание линий ESP, останавливая систему SpineIK");
					
					// Добавляем опцию отображения скелета
					ImGui::Checkbox("Bones", &Value::bools::Visuals::ESP::Bones);
					ImGui::SameLine();
					ImGui::ColorEdit4("##Bones", Value::Colors::Visuals::ESP::Bones, ImGuiColorEditFlags_NoInputs);
					
					// Добавляем опции для настройки предсказания и сглаживания костей
					if (Value::bools::Visuals::ESP::Bones) {
						ImGui::Checkbox("Bone Prediction", &Value::bools::Visuals::ESP::EnableBonePrediction);
						ImGui::SameLine();
						ImGui::TextDisabled("(?)");
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Предсказывает позицию костей при движении игрока");
							
						ImGui::SetNextItemWidth(120);
						ImGui::SliderFloat("Prediction Factor", &Value::floats::Visuals::ESP::BonePredictionFactor, 0.0f, 1.0f, "%.2f");
						ImGui::SameLine();
						ImGui::TextDisabled("(?)");
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Насколько сильно предсказывать движение (0-1)");
							
						ImGui::SetNextItemWidth(120);
						ImGui::SliderFloat("Smoothing Factor", &Value::floats::Visuals::ESP::BoneSmoothingFactor, 0.0f, 1.0f, "%.2f");
						ImGui::SameLine();
						ImGui::TextDisabled("(?)");
						if (ImGui::IsItemHovered())
							ImGui::SetTooltip("Насколько плавно сглаживать движение костей (0-1)");
					}
					
					ImGui::EndGroup();

					// Второй столбец
					ImGui::BeginGroup();
					ImGui::Checkbox("Weapon", &Value::bools::Visuals::ESP::Weapon);
					ImGui::SameLine();
					ImGui::ColorEdit4("##Weapon", Value::Colors::Visuals::ESP::Weapon, ImGuiColorEditFlags_NoInputs);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(100);
					ImGui::Combo("##WEAPON", &WEAPON, Items, IM_ARRAYSIZE(Items));

					ImGui::Checkbox("Distance", &Value::bools::Visuals::ESP::Distance);
					ImGui::SameLine();
					ImGui::ColorEdit4("##Distance", Value::Colors::Visuals::ESP::Distance, ImGuiColorEditFlags_NoInputs);
					ImGui::SameLine();
					ImGui::SetNextItemWidth(100);
					ImGui::Combo("##DISTANCE", &DISTANCE, Items, IM_ARRAYSIZE(Items));
					
					ImGui::Checkbox("Ignore Sleepers", &Value::bools::Visuals::ESP::IgnoreSleeper);
					ImGui::EndGroup();
					
					ImGui::Separator();
					
					ImGui::Text("Other ESP Features:");
					ImGui::Checkbox("Corpse", &Value::bools::Visuals::ESP::Corpse);
					ImGui::SameLine();
					ImGui::ColorEdit4("##Corpse", Value::Colors::Visuals::World::Corpse, ImGuiColorEditFlags_NoInputs);

					ImGui::Checkbox("Backpack", &Value::bools::Visuals::ESP::Backpack);
					ImGui::SameLine();
					ImGui::ColorEdit4("##Backpack", Value::Colors::Visuals::World::Backpack, ImGuiColorEditFlags_NoInputs);
				}
				
				ImGui::EndChild();
				ImGui::EndTabItem();
			}

			// Вкладка World
			if (ImGui::BeginTabItem("World"))
			{
				ImGui::BeginChild("World Settings", ImVec2(0, 0), true);
				
				ImGui::Columns(2, "WorldColumns", false);
				
				// Первый столбец
				ImGui::Checkbox("Stash", &Value::bools::Visuals::World::Items::Stash);
				ImGui::SameLine();
				ImGui::ColorEdit4("##Stash", Value::Colors::Visuals::World::Stash, ImGuiColorEditFlags_NoInputs);
				
				ImGui::Checkbox("Hemp", &Value::bools::Visuals::World::Items::Hemp);
				ImGui::SameLine();
				ImGui::ColorEdit4("##Hemp", Value::Colors::Visuals::World::Hemp, ImGuiColorEditFlags_NoInputs);
				
				ImGui::Text("Resource Nodes:");
				ImGui::Checkbox("Stone", &Value::bools::Visuals::World::Items::Ore::Stone);
				ImGui::SameLine();
				ImGui::ColorEdit4("##Stone", Value::Colors::Visuals::World::Ore::Stone, ImGuiColorEditFlags_NoInputs);
				
				ImGui::Checkbox("Iron", &Value::bools::Visuals::World::Items::Ore::Iron);
				ImGui::SameLine();
				ImGui::ColorEdit4("##Iron", Value::Colors::Visuals::World::Ore::Iron, ImGuiColorEditFlags_NoInputs);
				
				ImGui::Checkbox("Sulfur", &Value::bools::Visuals::World::Items::Ore::Sulfur);
				ImGui::SameLine();
				ImGui::ColorEdit4("##Sulfur", Value::Colors::Visuals::World::Ore::Sulfur, ImGuiColorEditFlags_NoInputs);
				
				ImGui::SetNextItemWidth(150);
				ImGui::SliderInt("Limit Distance", &Value::floats::Visuals::World::LimitDistance, 0, 400);
				
				ImGui::NextColumn();
				
				// Второй столбец
				ImGui::Text("Events & Vehicles:");
				ImGui::Checkbox("AirDrop", &Value::bools::Visuals::World::Items::AirDrop);
				ImGui::SameLine();
				ImGui::ColorEdit4("##AirDrop", Value::Colors::Visuals::World::AirDrop, ImGuiColorEditFlags_NoInputs);
				
				ImGui::Checkbox("CH47", &Value::bools::Visuals::World::Items::CH47);
				ImGui::SameLine();
				ImGui::ColorEdit4("##CH47", Value::Colors::Visuals::World::CH47, ImGuiColorEditFlags_NoInputs);
				
				ImGui::Checkbox("Minicopter", &Value::bools::Visuals::World::Items::Minicopter);
				ImGui::SameLine();
				ImGui::ColorEdit4("##Minicopter", Value::Colors::Visuals::World::Minicopter, ImGuiColorEditFlags_NoInputs);
				
				ImGui::Checkbox("Patrol", &Value::bools::Visuals::World::Items::Patrol);
				ImGui::SameLine();
				ImGui::ColorEdit4("##Patrol", Value::Colors::Visuals::World::Patrol, ImGuiColorEditFlags_NoInputs);
				
				ImGui::SetNextItemWidth(150);
				ImGui::SliderInt("Limit Distance (Far)", &Value::floats::Visuals::World::LimitDistance2, 0, 2000);
				
				ImGui::Columns(1);
				
				ImGui::Separator();
				
				ImGui::Text("Visual Settings:");
				ImGui::Checkbox("Always Day", &Value::bools::Visuals::World::AlwaysDay);
				if (Value::bools::Visuals::World::AlwaysDay) {
					ImGui::SameLine();
					ImGui::SetNextItemWidth(200);
					ImGui::SliderInt("Time", &Value::floats::Visuals::World::Time, 0, 24);
				}
				
				ImGui::EndChild();
				ImGui::EndTabItem();
			}

			// Вкладка Aim
			if (ImGui::BeginTabItem("Aim"))
			{
				ImGui::BeginChild("Aim Settings", ImVec2(0, 0), true);
				
				ImGui::Checkbox("Enable Aimbot", &Value::bools::Aim::Enable);
				
				if (Value::bools::Aim::Enable) {
					ImGui::Separator();
					
					ImGui::Columns(2, "AimColumns", false);
					
					// Первый столбец
					ImGui::Checkbox("Target Line", &Value::bools::Aim::TargetLine);
					ImGui::SameLine();
					ImGui::ColorEdit4("##TargetLine", Value::Colors::Aim::TargetLine, ImGuiColorEditFlags_NoInputs);
					
					ImGui::Checkbox("Visible Check", &Value::bools::Aim::VisibleCheck);
					ImGui::Checkbox("Ignore Team", &Value::bools::Aim::IgnoreTeam);
					ImGui::Checkbox("Ignore Sleepers", &Value::bools::Aim::IgnoreSleepers);
					
					ImGui::Checkbox("FOV Circle", &Value::bools::Aim::Fov);
					if (Value::bools::Aim::Fov) {
					ImGui::SameLine();
						ImGui::ColorEdit4("##Fov", Value::Colors::Aim::Fov, ImGuiColorEditFlags_NoInputs);
						ImGui::SetNextItemWidth(150);
						ImGui::SliderInt("FOV Size", &Value::floats::Aim::Fov, 0, 120);
					}
					
					ImGui::NextColumn();
					
					// Второй столбец
					ImGui::Checkbox("Smooth Aim", &Value::bools::Aim::Smooth);
					if (Value::bools::Aim::Smooth) {
						ImGui::SetNextItemWidth(150);
						ImGui::SliderFloat("Smooth Speed", &Value::floats::Aim::Smooth, 1, 25);
					}
					
					ImGui::Checkbox("Silent Aim", &Value::bools::Aim::SilentAim);
					if (Value::bools::Aim::SilentAim) {
						ImGui::SetNextItemWidth(150);
						ImGui::SliderFloat("Silent FOV", &Value::floats::Aim::SilentFov, 0, 180);
						ImGui::Checkbox("Silent Visible Check", &Value::bools::Aim::SilentVisibleCheck);
					}
					
					ImGui::Columns(1);
				}
				
				ImGui::EndChild();
				ImGui::EndTabItem();
			}

			// Вкладка Player
			if (ImGui::BeginTabItem("Player"))
			{
				ImGui::BeginChild("Player Settings", ImVec2(0, 0), true);
				
				ImGui::Columns(2, "PlayerColumns", false);
				
				// Первый столбец
				ImGui::Text("Movement Hacks:");
				ImGui::Checkbox("Spiderman", &Value::bools::Player::PlayerWalk::Spiderman);
				ImGui::Checkbox("Fake Admin", &Value::bools::Player::PlayerWalk::FakeAdmin);
				ImGui::Checkbox("Infinity Jump", &Value::bools::Player::PlayerWalk::InfinityJump);
				ImGui::Checkbox("Anti Aim", &Value::bools::Player::PlayerWalk::AntiAim);
				
				ImGui::Checkbox("Change Gravity", &Value::bools::Player::PlayerWalk::ChangeGravity);
				if (Value::bools::Player::PlayerWalk::ChangeGravity) {
					ImGui::SetNextItemWidth(150);
					ImGui::SliderFloat("Gravity Value", &Value::floats::Player::Gravity, 0, 2);
				}
				
				ImGui::NextColumn();
				
				// Второй столбец
				ImGui::Text("Visual Settings:");
				ImGui::Checkbox("Custom FOV", &Value::bools::Player::CustomFov);
				if (Value::bools::Player::CustomFov) {
					ImGui::SetNextItemWidth(150);
					ImGui::SliderFloat("FOV Value", &Value::floats::Player::CustomFov, 0, 120);
				}
				
				ImGui::Separator();
				
				ImGui::Text("Weapon Modifications:");
				ImGui::Checkbox("No Recoil", &Value::bools::Weapon::NoRecoil);
				ImGui::Checkbox("No Spread", &Value::bools::Weapon::NoSpread);
				ImGui::Checkbox("Thick Bullet", &Value::bools::Weapon::ThickBullet);
				ImGui::Checkbox("Force Automatic", &Value::bools::Weapon::IsAutomatic);
				
				ImGui::Columns(1);
				
				ImGui::EndChild();
				ImGui::EndTabItem();
			}

			// Вкладка Misc
			if (ImGui::BeginTabItem("Misc"))
			{
				ImGui::BeginChild("Misc Settings", ImVec2(0, 0), true);
				
				ImGui::Text("Player Info Panel:");
				ImGui::Checkbox("Enable Panel", &Value::bools::Visuals::PlayerPanel::Enable);
				if (Value::bools::Visuals::PlayerPanel::Enable) {
					ImGui::Checkbox("Show Name", &Value::bools::Visuals::PlayerPanel::Name);
					ImGui::Checkbox("Show HP", &Value::bools::Visuals::PlayerPanel::HP);
					ImGui::Checkbox("Show Weapons", &Value::bools::Visuals::PlayerPanel::Weapons);
				}
				
				ImGui::Separator();
				
				ImGui::Text("Radar Settings:");
				ImGui::Checkbox("Enable Radar", &Value::bools::Visuals::Radar::Enable);
				if (Value::bools::Visuals::Radar::Enable) {
					
					ImGui::SetNextItemWidth(150);
					ImGui::SliderInt("Radar Radius", &Value::floats::Visuals::Radar::Radius, 0, 200);
					
					ImGui::SetNextItemWidth(150);
					ImGui::SliderInt("Radar Distance", &Value::floats::Visuals::Radar::Distance, 0, 400);
				}
				
				ImGui::Separator();
				
				if (ImGui::Button("Reinitialize Cheat", ImVec2(180, 30))) {
					InitLocalPlayer();
				}
				
				ImGui::EndChild();
				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();

			ImGui::End();
		}

		Draw();

		bool valid = LocalPlayer.BasePlayer->IsValid(true);
		if (LocalPlayer.BasePlayer && valid) {


			std::lock_guard<std::mutex>lk(entity_mutex);
			entity_esp_thread();
			Misc(TodCycle);

			Aim(AimEntity);
		}
		else
			InitLocalPlayer();


		ImGui::PopFont();
		ImGui::EndFrame();
		D3dDevice->SetRenderState(D3DRS_ZENABLE, false);
		D3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
		D3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
		D3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(0, 0, 0, 0), 1.0f, 0);

		if (D3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			D3dDevice->EndScene();
		}
		HRESULT result = D3dDevice->Present(NULL, NULL, NULL, NULL);

		if (result == D3DERR_DEVICELOST && D3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			D3dDevice->Reset(&d3dpp);
			ImGui_ImplDX9_CreateDeviceObjects();
		}
	}
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(Window);
}

int main() {
	//Jopa();

	if (CreateConsole == false) { ShowWindow(::GetConsoleWindow(), SW_HIDE); }
	else { ShowWindow(::GetConsoleWindow(), SW_SHOW); }
	while (Initialised == false) {
		if (FindWindowA(NULL, TargetTitle)) {
			Initialised = true;
		}
	}

	while (hwnd == NULL)
	{
		printf("Please open Rust...");
		Sleep(50);
		system("cls");
		auto wind = "Rust";
		hwnd = FindWindowA(0, wind);
		Sleep(100);
	}
	pid = GetProcessIdByName("RustClient.exe");
	if (!pid) {
		std::cout << "RustClient not found";
		Sleep(10000);
		return 0;
	}
	oBaseAddress = GetModuleBaseAddress("GameAssembly.dll");

	InitLocalPlayer();
	Value::floats::Screen::W = GetSystemMetrics(SM_CXSCREEN);  // ���������� ������ �� �����������
	Value::floats::Screen::H = GetSystemMetrics(SM_CYSCREEN); // ���������� ������ �� ���������
	//SetupWindow();
	//DirectXInit(Wnd);
	//CreateThread(0, 0, ProcessCheck, 0, 0, 0);
	//CreateThread(0, 0, LoopThread, 0, 0, 0);
	CreateThread(0, 0, EntityThread, 0, 0, 0);
	//CreateThread(0, 0, InitLP, 0, 0, 0);

	//while (TRUE) {
	//	MainLoop();
	//}
	CreateOWindow();
	InitializeD3D();
	Loop();
	Shutdown();
	return 0;
}

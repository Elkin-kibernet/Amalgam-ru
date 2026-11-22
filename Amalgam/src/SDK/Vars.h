#pragma once
#include "../SDK/Definitions/Types.h"
#include "../Utils/Macros/Macros.h"
#include <windows.h>
#include <typeinfo>

#define DEFAULT_BIND -1

template <class T>
class ConfigVar;

class BaseVar
{
public:
	std::vector<const char*> m_vNames;
	int m_iFlags = 0;
	union {
		int i = 0;
		float f;
	} m_unMin;
	union {
		int i = 0;
		float f;
	} m_unMax;
	union {
		int i = 0;
		float f;
	} m_unStep;
	std::vector<const char*> m_vValues = {};
	const char* m_sExtra = nullptr;

protected:
	std::string m_sName;
	const char* m_sSection;

public:
	constexpr const char* Name() const
	{
		return m_sName.c_str();
	}
	constexpr const char* Section() const
	{
		return m_sSection;
	}

public:
	size_t m_iType;

	template <class T>
	inline ConfigVar<T>* As()
	{
		if (typeid(T).hash_code() != m_iType)
			return nullptr;

		return reinterpret_cast<ConfigVar<T>*>(this);
	}
};

namespace G
{
	inline std::vector<BaseVar*> Vars = {};
};

template <class T>
class ConfigVar : public BaseVar
{
public:
	T Value;
	T Default;
	std::unordered_map<int, T> Map = {};

	ConfigVar(T tValue, std::vector<const char*> vNames, const char* sName, const char* sSection, int iFlags = 0, std::vector<const char*> vValues = {}, const char* sNone = nullptr)
	{
		Value = Default = Map[DEFAULT_BIND] = tValue;
		m_iType = typeid(T).hash_code();

		m_vNames = vNames;
		m_sName = std::string(sName).replace(strlen(sName) - 1, 1, "");
		m_sSection = sSection;

		m_iFlags = iFlags;
		m_vValues = vValues;
		m_sExtra = sNone;

		G::Vars.push_back(this);
	}
	ConfigVar(T tValue, std::vector<const char*> vNames, const char* sName, const char* sSection, int iFlags, int iMin, int iMax, int iStep = 1, const char* sFormat = "%i")
	{
		Value = Default = Map[DEFAULT_BIND] = tValue;
		m_iType = typeid(T).hash_code();

		m_vNames = vNames;
		m_sName = std::string(sName).replace(strlen(sName) - 1, 1, "");
		m_sSection = sSection;

		m_iFlags = iFlags;
		m_unMin.i = iMin;
		m_unMax.i = iMax;
		m_unStep.i = iStep;
		m_sExtra = sFormat;

		G::Vars.push_back(this);
	}
	ConfigVar(T tValue, std::vector<const char*> vNames, const char* sName, const char* sSection, int iFlags, float flMin, float flMax, float flStep = 1.f, const char* sFormat = "%g")
	{
		Value = Default = Map[DEFAULT_BIND] = tValue;
		m_iType = typeid(T).hash_code();

		m_vNames = vNames;
		m_sName = std::string(sName).replace(strlen(sName) - 1, 1, "");
		m_sSection = sSection;

		m_iFlags = iFlags;
		m_unMin.f = flMin;
		m_unMax.f = flMax;
		m_unStep.f = flStep;
		m_sExtra = sFormat;

		G::Vars.push_back(this);
	}

	inline T& operator[](int i)
	{
		return Map[i];
	}
	inline bool contains(int i) const
	{
		return Map.contains(i);
	}
};

#define NAMESPACE_BEGIN(name, ...) \
	namespace name { \
		constexpr inline const char* Section() { return !std::string(#__VA_ARGS__).empty() ? ""#__VA_ARGS__ : #name; }
#define NAMESPACE_END(name) \
	}

#define CVar(name, title, value, ...) \
	constexpr inline const char* name##_() { return __FUNCTION__; } \
	inline ConfigVar<decltype(value)> name = { value, { title }, name##_(), Section(), __VA_ARGS__ }
#define CVarValues(name, title, value, flags, none, ...) \
	constexpr inline const char* name##_() { return __FUNCTION__; } \
	inline ConfigVar<decltype(value)> name = { value, { title }, name##_(), Section(), flags, { __VA_ARGS__ }, none }
#define Enum(name, ...) \
	namespace name##Enum { enum name##Enum { __VA_ARGS__ }; }
#define CVarEnum(name, title, value, flags, none, values, ...) \
	CVarValues(name, title, value, flags, none, values); \
	Enum(name, __VA_ARGS__);

#define NONE 0
#define VISUAL (1 << 31)
#define NOSAVE (1 << 30)
#define NOBIND (1 << 29)
#define DEBUGVAR (1 << 28)

// flags to be automatically used in widgets. keep these as the same values as the flags in components, do not include visual flags
#define SLIDER_CLAMP (1 << 2)
#define SLIDER_MIN (1 << 3)
#define SLIDER_MAX (1 << 4)
#define SLIDER_PRECISION (1 << 5)
#define SLIDER_NOAUTOUPDATE (1 << 6)
#define DROPDOWN_MULTI (1 << 2)
#define DROPDOWN_MODIFIABLE (1 << 3)
#define DROPDOWN_NOSANITIZATION (1 << 4)
#define DROPDOWN_CUSTOM (1 << 2)
#define DROPDOWN_AUTOUPDATE (1 << 3)

NAMESPACE_BEGIN(Vars)
	NAMESPACE_BEGIN(Menu)
		CVar(CheatTitle, "Название чита", std::string("Amalgam"), VISUAL | DROPDOWN_AUTOUPDATE);
		CVar(CheatTag, "Тег чита", std::string("[Amalgam]"), VISUAL);
		CVar(PrimaryKey, "Основная клавиша", VK_INSERT, NOBIND);
		CVar(SecondaryKey, "Дополнительная клавиша", VK_F3, NOBIND);

		CVar(BindWindow, "Окно биндов", true);
		CVar(BindWindowTitle, "Заголовок окна биндов", true);
		CVar(MenuShowsBinds, "Показывать бинды в меню", false, NOBIND);

		CVarEnum(Indicators, "Индикаторы", 0b00000, VISUAL | DROPDOWN_MULTI, nullptr,
			VA_LIST("Тики", "Критхак", "Наблюдатели", "Пинг", "Состояния", "Предикт сида"),
			Ticks = 1 << 0, CritHack = 1 << 1, Spectators = 1 << 2, Ping = 1 << 3, Conditions = 1 << 4, SeedPrediction = 1 << 5);

		CVar(BindsDisplay, "Бинды: позиция", DragBox_t(100, 100), VISUAL | NOBIND);
		CVar(TicksDisplay, "Тики: позиция", DragBox_t(), VISUAL | NOBIND);
		CVar(CritsDisplay, "Криты: позиция", DragBox_t(), VISUAL | NOBIND);
		CVar(SpectatorsDisplay, "Наблюдатели: позиция", DragBox_t(), VISUAL | NOBIND);
		CVar(PingDisplay, "Пинг: позиция", DragBox_t(), VISUAL | NOBIND);
		CVar(ConditionsDisplay, "Состояния: позиция", DragBox_t(), VISUAL | NOBIND);
		CVar(SeedPredictionDisplay, "Предикт сида: позиция", DragBox_t(), VISUAL | NOBIND);

		CVar(Scale, "Масштаб", 1.f, NOBIND | SLIDER_MIN | SLIDER_PRECISION | SLIDER_NOAUTOUPDATE, 0.75f, 2.f, 0.25f);
		CVar(CheapText, "Упрощённый текст", false, NOBIND);

		NAMESPACE_BEGIN(Theme)
			CVar(Accent, "Акцентный цвет", Color_t(175, 150, 255, 255), VISUAL);
			CVar(Background, "Цвет фона", Color_t(0, 0, 0, 250), VISUAL);
			CVar(Active, "Активный цвет", Color_t(255, 255, 255, 255), VISUAL);
			CVar(Inactive, "Неактивный цвет", Color_t(150, 150, 150, 255), VISUAL);
		NAMESPACE_END(Theme)
	NAMESPACE_END(Menu)

	NAMESPACE_BEGIN(Colors)
		CVar(IndicatorGood, "Индикатор: норм", Color_t(0, 255, 100, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorMid, "Индикатор: средне", Color_t(255, 200, 0, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorBad, "Индикатор: плохо", Color_t(255, 0, 0, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorMisc, "Индикатор: прочее", Color_t(75, 175, 255, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorTextGood, "Текст индикатора: норм", Color_t(150, 255, 150, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorTextMid, "Текст индикатора: средне", Color_t(255, 200, 0, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorTextBad, "Текст индикатора: плохо", Color_t(255, 150, 150, 255), NOSAVE | DEBUGVAR);
		CVar(IndicatorTextMisc, "Текст индикатора: прочее", Color_t(100, 255, 255, 255), NOSAVE | DEBUGVAR);

		CVar(Local, "Цвет локального", Color_t(255, 255, 255), VISUAL);
		CVar(FOVCircle, "Цвет круга FOV", Color_t(255, 255, 255, 100), VISUAL);
		CVar(SpellFootstep, "Цвет следов заклинания", Color_t(255, 255, 255, 255), VISUAL);

		CVar(WorldModulation, VA_LIST("Модуляция мира", "Цвет модуляции мира"), Color_t(255, 255, 255, 255), VISUAL);
		CVar(SkyModulation, VA_LIST("Модуляция неба", "Цвет модуляции неба"), Color_t(255, 255, 255, 255), VISUAL);
		CVar(PropModulation, VA_LIST("Модуляция объектов", "Цвет модуляции объектов"), Color_t(255, 255, 255, 255), VISUAL);
		CVar(ParticleModulation, VA_LIST("Модуляция частиц", "Цвет модуляции частиц"), Color_t(255, 255, 255, 255), VISUAL);
		CVar(FogModulation, VA_LIST("Модуляция тумана", "Цвет модуляции тумана"), Color_t(255, 255, 255, 255), VISUAL);

		CVar(Line, "Цвет линии", Color_t(255, 255, 255, 255), VISUAL);
		CVar(LineIgnoreZ, "Цвет линии сквозь стены", Color_t(255, 255, 255, 0), VISUAL);

		CVar(BoneHitboxEdge, "Кость: цвет ребра", Color_t(255, 255, 255, 255), VISUAL);
		CVar(BoneHitboxEdgeIgnoreZ, "Кость: ребро сквозь стены", Color_t(255, 255, 255, 0), VISUAL);
		CVar(BoneHitboxFace, "Кость: цвет грани", Color_t(255, 255, 255, 0), VISUAL);
		CVar(BoneHitboxFaceIgnoreZ, "Кость: грань сквозь стены", Color_t(255, 255, 255, 0), VISUAL);
		CVar(TargetHitboxEdge, "Цель: цвет ребра", Color_t(255, 150, 150, 255), VISUAL);
		CVar(TargetHitboxEdgeIgnoreZ, "Цель: ребро сквозь стены", Color_t(255, 150, 150, 0), VISUAL);
		CVar(TargetHitboxFace, "Цель: цвет грани", Color_t(255, 150, 150, 0), VISUAL);
		CVar(TargetHitboxFaceIgnoreZ, "Цель: грань сквозь стены", Color_t(255, 150, 150, 0), VISUAL);
		CVar(BoundHitboxEdge, "Обводка: цвет ребра", Color_t(255, 255, 255, 255), VISUAL);
		CVar(BoundHitboxEdgeIgnoreZ, "Обводка: ребро сквозь стены", Color_t(255, 255, 255, 0), VISUAL);
		CVar(BoundHitboxFace, "Обводка: цвет грани", Color_t(255, 255, 255, 0), VISUAL);
		CVar(BoundHitboxFaceIgnoreZ, "Обводка: грань сквозь стены", Color_t(255, 255, 255, 0), VISUAL);

		CVar(PlayerPath, "Цвет пути игрока", Color_t(255, 255, 255, 0), VISUAL);
		CVar(PlayerPathIgnoreZ, "Путь игрока сквозь стены", Color_t(255, 255, 255, 255), VISUAL);
		CVar(ProjectilePath, "Цвет пути снаряда", Color_t(255, 255, 255, 0), VISUAL);
		CVar(ProjectilePathIgnoreZ, "Путь снаряда сквозь стены", Color_t(255, 255, 255, 255), VISUAL);
		CVar(TrajectoryPath, "Цвет траектории", Color_t(255, 255, 255, 0), VISUAL);
		CVar(TrajectoryPathIgnoreZ, "Траектория сквозь стены", Color_t(255, 255, 255, 255), VISUAL);
		CVar(ShotPath, "Цвет пути выстрела", Color_t(255, 255, 255, 0), VISUAL);
		CVar(ShotPathIgnoreZ, "Путь выстрела сквозь стены", Color_t(255, 255, 255, 255), VISUAL);
		CVar(SplashRadius, "Цвет радиуса сплеша", Color_t(255, 255, 255, 0), VISUAL);
		CVar(SplashRadiusIgnoreZ, "Радиус сплеша сквозь стены", Color_t(255, 255, 255, 255), VISUAL);
		CVar(RealPath, "Цвет реального пути", Color_t(255, 255, 255, 0), NOSAVE | DEBUGVAR);
		CVar(RealPathIgnoreZ, "Реальный путь сквозь стены", Color_t(255, 255, 255, 255), NOSAVE | DEBUGVAR);
	NAMESPACE_END(Colors)

	NAMESPACE_BEGIN(Aimbot)
		NAMESPACE_BEGIN(General, Aimbot)
			CVarEnum(AimType, "Тип аима", 0, NONE, nullptr,
				VA_LIST("Выкл", "Обычный", "Плавный", "Сайлент", "Захват", "Помощь"),
				Off, Plain, Smooth, Silent, Locking, Assistive);
			CVarEnum(TargetSelection, "Выбор цели", 0, NONE, nullptr,
				VA_LIST("FOV", "Дистанция", "Гибрид"),
				FOV, Distance, Hybrid);
			CVarEnum(Target, "Цели", 0b0000001, DROPDOWN_MULTI, nullptr,
				VA_LIST("Игроки", "Турели", "Диспенсеры", "Телепорты", "Стики", "NPC", "Бомбы"),
				Players = 1 << 0, Sentry = 1 << 1, Dispenser = 1 << 2, Teleporter = 1 << 3, Stickies = 1 << 4, NPCs = 1 << 5, Bombs = 1 << 6,
				All = Players | Sentry | Dispenser | Teleporter | Stickies | NPCs | Bombs, Building = Sentry | Dispenser | Teleporter);
			CVarEnum(Ignore, "Игнорировать", 0b00000001000, DROPDOWN_MULTI, nullptr,
				VA_LIST("Друзья", "Патька", "Без приоритета", "Неуязвимые", "Невидимые", "Без симуляции", "Дэд-рингер", "Вакцинатор", "Замаскированные", "Таунтят", "Свои"),
				Friends = 1 << 0, Party = 1 << 1, Unprioritized = 1 << 2, Invulnerable = 1 << 3, Invisible = 1 << 4, Unsimulated = 1 << 5, DeadRinger = 1 << 6, Vaccinator = 1 << 7, Disguised = 1 << 8, Taunting = 1 << 9, Team = 1 << 10);
			CVar(AimFOV, "FOV аима", 180.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 180.f);
			CVar(MaxTargets, "Макс. целей", 2, SLIDER_MIN, 1, 6);
			CVar(IgnoreInvisible, "Игнор. невидимых", 50.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 10.f, "%g%%");
			CVar(AssistStrength, "Сила помощи", 25.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 1.f, "%g%%");
			CVar(TickTolerance, "Допуск тиков", 4, SLIDER_CLAMP, 0, 21);
			CVar(AutoShoot, "Автострельба", true);
			CVar(FOVCircle, "Круг FOV", true, VISUAL);
			CVar(LeadAndRestrict, "Упреждение и ограничение", false, VISUAL);
			CVar(NoSpread, "Без разброса", false);

			CVarEnum(AimHoldsFire, "Аим держит огонь", 2, NOSAVE | DEBUGVAR, nullptr,
				VA_LIST("Нет", "Только миниган", "Всегда"),
				False, MinigunOnly, Always);
			CVar(NoSpreadOffset, "Смещение без разброса", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -1.f, 1.f, 0.1f);
			CVar(NoSpreadAverage, "Усреднение без разброса", 5, NOSAVE | DEBUGVAR | SLIDER_MIN, 1, 25);
			CVar(NoSpreadInterval, "Интервал без разброса", 0.1f, NOSAVE | DEBUGVAR | SLIDER_MIN, 0.05f, 5.f, 0.1f, "%gs");
			CVar(NoSpreadBackupInterval, "Резервный интервал", 2.f, NOSAVE | DEBUGVAR | SLIDER_MIN, 2.f, 10.f, 0.1f, "%gs");
		NAMESPACE_END(Global)

		NAMESPACE_BEGIN(Hitscan)
			CVarEnum(Hitboxes, VA_LIST("Хитбоксы", "Хитбоксы хитскана"), 0b000111, DROPDOWN_MULTI, nullptr,
				VA_LIST("Голова", "Тело", "Таз", "Руки", "Ноги", "##Divider", "Боди-аим если убьёт", "Только в голову"),
				Head = 1 << 0, Body = 1 << 1, Pelvis = 1 << 2, Arms = 1 << 3, Legs = 1 << 4, BodyaimIfLethal = 1 << 5, HeadshotOnly = 1 << 6);
			CVarValues(MultipointHitboxes, "Мультипоинт хитбоксы", 0b00000, DROPDOWN_MULTI, "Все",
				VA_LIST("Голова", "Тело", "Таз", "Руки", "Ноги"));
			CVarEnum(Modifiers, VA_LIST("Модификаторы", "Модификаторы хитскана"), 0b0100000, DROPDOWN_MULTI, nullptr,
				VA_LIST("Тапфайр", "Ждать хедшот", "Ждать заряд", "Только в прицеле", "Автоприцел", "Автораскрутка минигана", "Тушить своих"),
				Tapfire = 1 << 0, WaitForHeadshot = 1 << 1, WaitForCharge = 1 << 2, ScopedOnly = 1 << 3, AutoScope = 1 << 4, AutoRev = 1 << 5, ExtinguishTeam = 1 << 6);
			CVar(MultipointScale, "Масштаб мультипоинта", 0.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 5.f, "%g%%");
			CVar(TapfireDistance, "Дистанция тапфайра", 1000.f, SLIDER_MIN | SLIDER_PRECISION, 250.f, 1000.f, 50.f);

			CVarEnum(PeekCheck, "Проверка пика", 1, NOSAVE | DEBUGVAR, nullptr,
				VA_LIST("Выкл", "Только даблтап", "Всегда"),
				Off, DoubletapOnly, Always);
			CVar(PeekAmount, "Кол-во проверок пика", 1, NOSAVE | DEBUGVAR, 0, 5);
			CVar(BoneSizeSubtract, "Вычет размера кости", 1.f, NOSAVE | DEBUGVAR | SLIDER_MIN, 0.f, 4.f, 0.25f);
			CVar(BoneSizeMinimumScale, "Мин. масштаб кости", 1.f, NOSAVE | DEBUGVAR | SLIDER_CLAMP, 0.f, 1.f, 0.1f);
		NAMESPACE_END(HITSCAN)

		NAMESPACE_BEGIN(Projectile)
			CVarEnum(StrafePrediction, VA_LIST("Предикт", "Предикт стрейфа"), 0b11, DROPDOWN_MULTI, "Выкл",
				VA_LIST("Стрейф в воздухе", "Стрейф на земле"),
				Air = 1 << 0, Ground = 1 << 1);
			CVarEnum(Hitboxes, VA_LIST("Хитбоксы", "Хитбоксы снарядов"), 0b001111, DROPDOWN_MULTI, nullptr,
				VA_LIST("Авто", "##Divider", "Голова", "Тело", "Ступни", "##Divider", "Боди-аим если убьёт", "Приоритет ступней"),
				Auto = 1 << 0, Head = 1 << 1, Body = 1 << 2, Feet = 1 << 3, BodyaimIfLethal = 1 << 4, PrioritizeFeet = 1 << 5);
			CVarEnum(SplashPrediction, "Предикт сплеша", 0, NONE, nullptr,
				VA_LIST("Выкл", "Включать", "Предпочитать", "Только"),
				Off, Include, Prefer, Only);
			CVarEnum(SplashMode, "Режим сплеша", 0, NONE, nullptr,
				VA_LIST("Трейс", "Грань"),
				Trace, Face);
			CVarEnum(AutoDetonate, "Автоподрыв", 0b00, DROPDOWN_MULTI, "Выкл",
				VA_LIST("Стики", "Фейерверки", "##Divider", "Без урона себе", "Ignore invisible"),
				Stickies = 1 << 0, Flares = 1 << 1, PreventSelfDamage = 1 << 2, IgnoreInvisible = 1 << 3);
			CVarEnum(AutoAirblast, "Автовоздух", 0b000, DROPDOWN_MULTI, "Выкл",
				VA_LIST("Enabled", "##Divider", "Редирект", "Игнор. FOV"),
				Enabled = 1 << 0, Redirect = 1 << 1, IgnoreFOV = 1 << 2);
			CVarEnum(Modifiers, VA_LIST("Модификаторы", "Модификаторы снарядов"), 0b0010, DROPDOWN_MULTI, nullptr,
				VA_LIST("Заряжать оружие", "Отменять заряд", "Учитывать взвод", "Сплеш в воздухе", "Углы навеса", "Цели в простое"),
				ChargeWeapon = 1 << 0, CancelCharge = 1 << 1, UseArmTime = 1 << 2, AirSplash = 1 << 3, LobAngles = 1 << 4, TargetDormant = 1 << 5);
			CVar(MaxSimulationTime, "Макс. время симуляции", 1.f, SLIDER_MIN | SLIDER_PRECISION, 0.1f, 2.5f, 0.25f, "%gs");
			CVar(HitChance, "Шанс попадания", 0.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 10.f, "%g%%");
			CVar(AutodetRadius, "Радиус автоподрыва", 90.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 10.f, "%g%%");
			CVar(SplashRadius, "Радиус сплеша", 90.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 10.f, "%g%%");
			CVar(AutoRelease, "Автовыпуск", 0.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 5.f, "%g%%");

			CVar(GroundSamples, "Сэмплы", 33, NOSAVE | DEBUGVAR, 3, 66);
			CVar(GroundStraightFuzzyValue, "Порог прямого движения", 100.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 500.f, 25.f);
			CVar(GroundLowMinimumSamples, "Мин. сэмплов (низко)", 16, NOSAVE | DEBUGVAR, 3, 66);
			CVar(GroundHighMinimumSamples, "Мин. сэмплов (высоко)", 33, NOSAVE | DEBUGVAR, 3, 66);
			CVar(GroundLowMinimumDistance, "Мин. дистанция (низко)", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 2500.f, 100.f);
			CVar(GroundHighMinimumDistance, "Мин. дистанция (высоко)", 1000.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 2500.f, 100.f);
			CVar(GroundMaxChanges, "Макс. изменений", 0, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0, 5);
			CVar(GroundMaxChangeTime, "Макс. время изменения", 0, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0, 66);

			CVar(AirSamples, "Сэмплы", 33, NOSAVE | DEBUGVAR, 3, 66);
			CVar(AirStraightFuzzyValue, "Порог прямого движения", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 500.f, 25.f);
			CVar(AirLowMinimumSamples, "Мин. сэмплов (низко)", 16, NOSAVE | DEBUGVAR, 3, 66);
			CVar(AirHighMinimumSamples, "Мин. сэмплов (высоко)", 16, NOSAVE | DEBUGVAR, 3, 66);
			CVar(AirLowMinimumDistance, "Мин. дистанция (низко)", 100000.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 2500.f, 100.f);
			CVar(AirHighMinimumDistance, "Мин. дистанция (высоко)", 100000.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 2500.f, 100.f);
			CVar(AirMaxChanges, "Макс. изменений", 2, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0, 5);
			CVar(AirMaxChangeTime, "Макс. время изменения", 16, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0, 66);

			CVar(VelocityAverageCount, "Окно усреднения скорости", 5, NOSAVE | DEBUGVAR, 1, 10);
			CVar(VerticalShift, "Вертикальный сдвиг", 5.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f, 0.5f);
			CVar(DragOverride, "Переопределение сопротивления", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 1.f, 0.01f);
			CVar(TimeOverride, "Переопределение времени", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 2.f, 0.01f);
			CVar(LobAnglesUnderpredict, "Недопредикт углов навеса", true, NOSAVE | DEBUGVAR);
			CVar(HuntsmanLerp, "Интерполяция охотника", 50.f, NOSAVE | DEBUGVAR | SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 1.f, "%g%%");
			CVar(HuntsmanLerpLow, "Интерполяция охотника (низко)", 100.f, NOSAVE | DEBUGVAR | SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 100.f, 1.f, "%g%%");
			CVar(HuntsmanAdd, "Поправка охотника", 0.f, NOSAVE | DEBUGVAR | SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 20.f);
			CVar(HuntsmanAddLow, "Поправка охотника (низко)", 0.f, NOSAVE | DEBUGVAR | SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 20.f);
			CVar(HuntsmanClamp, "Ограничение охотника", 5.f, NOSAVE | DEBUGVAR | SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 10.f, 0.5f);
			CVar(HuntsmanPullPoint, "Точка натяжения охотника", false, NOSAVE | DEBUGVAR);
			CVar(HuntsmanPullNoZ, "Натяжение без Z", false, NOSAVE | DEBUGVAR);

			CVar(SplashAirCount, "Сплеш: возд. сэмплы", 0, NOSAVE | DEBUGVAR | SLIDER_MIN, 0, 10);
			CVar(SplashPointsDirect, "Сплеш: прямые точки", 100, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0, 400, 5);
			CVar(SplashPointsArc, "Сплеш: дуговые точки", 100, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0, 400, 5);
			CVar(SplashRotateX, "Сплеш: поворот X", -1.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, -1.f, 360.f);
			CVar(SplashRotateY, "Сплеш: поворот Y", -1.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, -1.f, 360.f);
			CVar(SplashDensityDirect, "Сплеш: плотность прямая", 40.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 100.f, 1.f);
			CVar(SplashDensityArc, "Сплеш: плотность дуги", 40.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 100.f, 1.f);
			CVar(SplashSamplesCutoff, "Сплеш: отсечение сэмплов", 0.0000001f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 0.000001f, 0.00000001f);
			CVar(SplashRestrictDirect, "Сплеш: лимит прямых", 100, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 1, 400, 5);
			CVar(SplashRestrictArc, "Сплеш: лимит дуги", 5, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 1, 400, 5);
			CVar(SplashRestrictFirst, "Сплеш: лимит первых", 25, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 1, 400, 5);
			CVar(DirectTraceInterval, "Интервал прямых трейсов", 1, NOSAVE | DEBUGVAR | SLIDER_MIN, 1, 20);
			CVar(SplashTraceInterval, "Интервал трейсов сплеша", 10, NOSAVE | DEBUGVAR | SLIDER_MIN, 1, 20);
			CVar(LobTraceInterval, "Интервал трейсов навеса", 20, NOSAVE | DEBUGVAR | SLIDER_MIN, 1, 20);
			CVar(IntervalRetest, "Повторная проверка интервала", true, NOSAVE | DEBUGVAR);

			CVar(DeltaCount, "Окно дельты", 5, NOSAVE | DEBUGVAR, 1, 5);
			CVarEnum(DeltaMode, "Режим дельты", 0, NOSAVE | DEBUGVAR, nullptr,
				VA_LIST("Среднее", "Максимум"),
				Average, Max);
			CVarEnum(MovesimFrictionFlags, "Флаги трения мовсима", 0b01, NOSAVE | DEBUGVAR | DROPDOWN_MULTI, nullptr,
				VA_LIST("Уменьшение бега", "Расчёт роста"),
				RunReduce = 1 << 0, CalculateIncrease = 1 << 1);
		NAMESPACE_END(Projectile)

		NAMESPACE_BEGIN(Melee)
			CVar(AutoBackstab, "Автоудар в спину", true);
			CVar(IgnoreRazorback, "Игнор. рэйзорбек", false);
			CVar(SwingPrediction, "Предикт удара", true);
			CVar(WhipTeam, "Хлыст по своим", false);

			CVar(SwingTicks, "Тики удара", 13, NOSAVE | DEBUGVAR | SLIDER_MIN, 0, 14);
			CVar(SwingPredictLag, "Предикт задержки удара", true, NOSAVE | DEBUGVAR);
			CVarEnum(SwingValidateMode, "Режим проверки удара", 0, NOSAVE | DEBUGVAR, nullptr,
				VA_LIST("Оба", "Удар", "Симуляция"),
				Both, Swing, Simulated);
			CVarEnum(BackstabFlags, "Флаги удара в спину", 0b11, NOSAVE | DEBUGVAR | DROPDOWN_MULTI, nullptr,
				VA_LIST("Учитывать пинг", "Двойная проверка"),
				AccountPing = 1 << 0, DoubleTest = 1 << 1);
		NAMESPACE_END(Melee)

		NAMESPACE_BEGIN(Healing)
			CVarEnum(HealPriority, "Приоритет лечения", 0, NONE, nullptr,
				VA_LIST("Нет", "Приоритет своим", "Приоритет друзьям", "Только друзья"),
				None, PrioritizeTeam, PrioritizeFriends, FriendsOnly);
			CVarEnum(DangerIgnore, "Игнор. опасных", 0b1000, DROPDOWN_MULTI, nullptr,
				VA_LIST("Друзья", "Патька", "Без приоритета", "Игнор."),
				Friends = 1 << 0, Party = 1 << 1, Unprioritized = 1 << 2, Ignored = 1 << 3,
				Shared = Friends | Party | Unprioritized);
			CVar(AutoHeal, "Автолечение", false);
			CVar(AutoArrow, "Автострела", false);
			CVar(AutoRepair, "Авторемонт", false);
			CVar(AutoSandvich, "Автосэндвич", false);
			CVar(AutoVaccinator, "Автовакцинатор", false);
			CVar(ActivateOnVoice, "Активация по голосу", false);

			CVar(AutoVaccinatorBulletScale, "Вакцинатор: пули", 100.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 200.f, 10.f, "%g%%");
			CVar(AutoVaccinatorBlastScale, "Вакцинатор: взрыв", 100.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 200.f, 10.f, "%g%%");
			CVar(AutoVaccinatorFireScale, "Вакцинатор: огонь", 100.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 200.f, 10.f, "%g%%");
			CVar(AutoVaccinatorFlamethrowerDamageOnly, "Вакцинатор: только урон огнемёта", false, NOSAVE | DEBUGVAR);
		NAMESPACE_END(Healing)
	NAMESPACE_END(Aimbot)
	
	NAMESPACE_BEGIN(CritHack, Crit Hack)
		CVar(ForceCrits, "Форс критов", false);
		CVar(AvoidRandomCrits, "Убирать рандом-криты", false);
		CVar(AlwaysMeleeCrit, "Всегда крит в ближнем бою", false);
		CVar(CritEffects, "Эффекты критов", false);
	NAMESPACE_END(CritHack)

	NAMESPACE_BEGIN(Backtrack)
		CVar(Latency, "Фейковый пинг", 0, SLIDER_CLAMP, 0, 1000, 5);
		CVar(Interp, "Фейковый интерп", 0, SLIDER_CLAMP | SLIDER_PRECISION, 0, 1000, 5);
		CVar(Window, VA_LIST("Окно", "Окно бэктрека"), 185, SLIDER_CLAMP | SLIDER_PRECISION, 0, 200, 5);
		CVar(PreferOnShot, "Предпочитать при выстреле", false);

		CVar(Offset, "Смещение", 0, NOSAVE | DEBUGVAR, -1, 1);
	NAMESPACE_END(Backtrack)

	NAMESPACE_BEGIN(Doubletap)
		CVar(Doubletap, "Даблтап", false);
		CVar(Warp, "Варп", false);
		CVar(RechargeTicks, "Тики перезарядки", false);
		CVar(AntiWarp, "Анти-варп", true);
		CVar(TickLimit, "Лимит тиков", 22, SLIDER_CLAMP, 2, 22);
		CVar(WarpRate, "Скорость варпа", 22, SLIDER_CLAMP, 2, 22);
		CVar(RechargeLimit, "Лимит перезарядки", 24, SLIDER_MIN, 1, 24);
		CVar(PassiveRecharge, "Пассивная перезарядка", 0, SLIDER_CLAMP, 0, 67);
	NAMESPACE_END(DoubleTap)

	NAMESPACE_BEGIN(Fakelag)
		CVarEnum(Fakelag, "Фейклаг", 0, NONE, nullptr,
			VA_LIST("Выкл", "Обычный", "Случайный", "Адаптивный"),
			Off, Plain, Random, Adaptive);
		CVarEnum(Options, VA_LIST("Опции", "Опции фейклага"), 0b000, DROPDOWN_MULTI, nullptr,
			VA_LIST("Только в движении", "При отпускании приседа", "Не в воздухе"),
			OnlyMoving = 1 << 0, OnUnduck = 1 << 1, NotAirborne = 1 << 2);
		CVar(PlainTicks, "Обычные тики", 12, SLIDER_CLAMP, 1, 22);
		CVar(RandomTicks, "Случайные тики", IntRange_t(14, 18), SLIDER_CLAMP, 1, 22, 1, "%i - %i");
		CVar(UnchokeOnAttack, "Разжатие при атаке", true);
		CVar(RetainBlastJump, "Сохранять бластджамп", false);

		CVar(RetainSoldierOnly, "Бластджамп только за солдата", true, NOSAVE | DEBUGVAR);
	NAMESPACE_END(FakeLag)

	NAMESPACE_BEGIN(AutoPeek, Auto Peek)
		CVar(Enabled, VA_LIST("Включено", "Автопик"), false);
	NAMESPACE_END(AutoPeek)

	NAMESPACE_BEGIN(Speedhack)
		CVar(Scale, VA_LIST("Масштаб", "Множитель спидхака"), 1, NONE, 1, 50);
	NAMESPACE_END(Speedhack)

	NAMESPACE_BEGIN(AntiAim, Antiaim)
		CVar(Enabled, VA_LIST("Включено", "Анти-аим включён"), false);
		CVarEnum(PitchReal, "Реальный питч", 0, NONE, nullptr,
			VA_LIST("Нет", "Вверх", "Вниз", "Ноль", "Джиттер", "Реверс-джиттер"),
			None, Up, Down, Zero, Jitter, ReverseJitter);
		CVarEnum(PitchFake, "Фейковый питч", 0, NONE, nullptr,
			VA_LIST("Нет", "Вверх", "Вниз", "Джиттер", "Реверс-джиттер"),
			None, Up, Down, Jitter, ReverseJitter);
		Enum(Yaw, Forward, Left, Right, Backwards, Edge, Jitter, Spin);
		CVarValues(YawReal, "Реальный йоу", 0, NONE, nullptr,
			"Вперёд", "Влево", "Вправо", "Назад", "Край", "Джиттер", "Вращение");
		CVarValues(YawFake, "Фейковый йоу", 0, NONE, nullptr,
			"Вперёд", "Влево", "Вправо", "Назад", "Край", "Джиттер", "Вращение");
		Enum(YawMode, View, Target);
		CVarValues(RealYawBase, "Реальная база", 0, NONE, nullptr,
			"Вид", "Цель");
		CVarValues(FakeYawBase, "Фейковая база", 0, NONE, nullptr,
			"Вид", "Цель");
		CVar(RealYawOffset, "Реальное смещение", 0.f, SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
		CVar(FakeYawOffset, "Фейковое смещение", 0.f, SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
		CVar(RealYawValue, "Реальное значение", 90.f, SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
		CVar(FakeYawValue, "Фейковое значение", -90.f, SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
		CVar(SpinSpeed, "Скорость вращения", 15.f, SLIDER_PRECISION, -30.f, 30.f);
		CVar(MinWalk, "Минволк", true);
		CVar(HidePitchOnShot, "Скрывать питч при выстреле", false);

		CVar(AntiAimLines, "Линии анти-аима", false, NOSAVE);
	NAMESPACE_END(AntiAim)

	NAMESPACE_BEGIN(Resolver)
		CVar(Enabled, VA_LIST("Включено", "Резолвер включён"), false);
		CVar(AutoResolve, "Авторезолв", false);
		CVar(AutoResolveCheatersOnly, "Авторезолв только читеров", false);
		CVar(AutoResolveHeadshotOnly, "Авторезолв только в голову", false);
		CVar(AutoResolveYawAmount, "Авторезолв: йоу", 90.f, SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 45.f);
		CVar(AutoResolvePitchAmount, "Авторезолв: питч", 90.f, SLIDER_CLAMP, -180.f, 180.f, 90.f);
		CVar(CycleYaw, "Циклический йоу", 0.f, SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 45.f);
		CVar(CyclePitch, "Циклический питч", 0.f, SLIDER_CLAMP, -180.f, 180.f, 90.f);
		CVar(CycleView, "Циклический вид", false);
		CVar(CycleMinwalk, "Циклический минволк", false);
	NAMESPACE_END(Resolver)

	NAMESPACE_BEGIN(ESP)
		CVarValues(ActiveGroups, "Активные группы", int(0b11111111111111111111111111111111), VISUAL | DROPDOWN_MULTI | DROPDOWN_NOSANITIZATION, nullptr);
	NAMESPACE_END(ESP)

	NAMESPACE_BEGIN(Visuals)
		NAMESPACE_BEGIN(UI)
			CVarEnum(StreamerMode, "Стример-режим", 0, VISUAL, nullptr,
				VA_LIST("Выкл", "Локальный", "Друзья", "Патька", "Все"),
				Off, Local, Friends, Party, All);
			CVarEnum(ChatTags, "Теги в чате", 0b000, VISUAL | DROPDOWN_MULTI, nullptr,
				VA_LIST("Локальный", "Друзья", "Патька", "Назначенные"),
				Local = 1 << 0, Friends = 1 << 1, Party = 1 << 2, Assigned = 1 << 3);
			CVar(FieldOfView, "Поле зрения## FOV", 0.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 160.f, 5.f);
			CVar(ZoomFieldOfView, "Поле зрения в зуме## Zoomed FOV", 0.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 160.f, 5.f);
			CVar(AspectRatio, "Соотношение сторон", 0.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 5.f, 0.05f);
			CVar(RevealScoreboard, "Раскрывать таблицу", false, VISUAL);
			CVar(ScoreboardUtility, "Утилита таблицы", false);
			CVar(ScoreboardColors, "Цвета таблицы", false, VISUAL);
			CVar(CleanScreenshots, "Чистые скриншоты", true);
		NAMESPACE_END(UI)

		NAMESPACE_BEGIN(Thirdperson)
			CVar(Enabled, "Третье лицо", false, VISUAL);
			CVar(Crosshair, VA_LIST("Прицел", "Прицел от третьего лица"), false, VISUAL);
			CVar(Distance, "Дистанция третьего лица", 150.f, VISUAL | SLIDER_PRECISION, 0.f, 400.f, 10.f);
			CVar(Right, "Смещение вправо", 0.f, VISUAL | SLIDER_PRECISION, -100.f, 100.f, 5.f);
			CVar(Up, "Смещение вверх", 0.f, VISUAL | SLIDER_PRECISION, -100.f, 100.f, 5.f);

			CVar(Scale, "Масштаб третьего лица", true, NOSAVE | DEBUGVAR);
			CVar(Collide, "Коллизия третьего лица", true, NOSAVE | DEBUGVAR);
		NAMESPACE_END(ThirdPerson)

		NAMESPACE_BEGIN(Removals)
			CVar(Interpolation, VA_LIST("Интерполяция", "Убрать интерполяцию"), false);
			CVar(Lerp, VA_LIST("Лерп", "Убрать лерп"), true);
			CVar(Disguises, VA_LIST("Маскировки", "Убрать маскировки"), false, VISUAL);
			CVar(Taunts, VA_LIST("Таунты", "Убрать таунты"), false, VISUAL);
			CVar(Scope, VA_LIST("Прицел снайперки", "Убрать прицел"), false, VISUAL);
			CVar(PostProcessing, VA_LIST("Постобработка", "Убрать постобработку"), false, VISUAL);
			CVar(ScreenOverlays, VA_LIST("Экранные оверлеи", "Убрать оверлеи"), false, VISUAL);
			CVar(ScreenEffects, VA_LIST("Экранные эффекты", "Убрать эффекты"), false, VISUAL);
			CVar(ViewPunch, VA_LIST("Отдача камеры", "Убрать отдачу камеры"), false, VISUAL);
			CVar(AngleForcing, VA_LIST("Форс углов", "Убрать форс углов"), false, VISUAL);
			CVar(Ragdolls, VA_LIST("Рэгдоллы", "Убрать рэгдоллы"), false, VISUAL);
			CVar(Gibs, VA_LIST("Гибы", "Убрать гибы"), false, VISUAL);
			CVar(MOTD, VA_LIST("MOTD", "Убрать MOTD"), false, VISUAL);
		NAMESPACE_END(Removals)

		NAMESPACE_BEGIN(Effects)
			CVarValues(BulletTracer, "Трейсер пуль", std::string("Default"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				"Default", "None", "Big nasty", "Distortion trail", "Machina", "Sniper rail", "Short circuit", "C.A.P.P.E.R", "Merasmus ZAP", "Merasmus ZAP 2", "Black ink", "Line", "Line ignore Z", "Beam");
			CVarValues(CritTracer, "Трейсер критов", std::string("Default"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				"Default", "None", "Big nasty", "Distortion trail", "Machina", "Sniper rail", "Short circuit", "C.A.P.P.E.R", "Merasmus ZAP", "Merasmus ZAP 2", "Black ink", "Line", "Line ignore Z", "Beam");
			CVarValues(MedigunBeam, "Луч медгана", std::string("Default"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				"Default", "None", "Uber", "Dispenser", "Passtime", "Bombonomicon", "White", "Orange");
			CVarValues(MedigunCharge, "Заряд медгана", std::string("Default"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				"Default", "None", "Electrocuted", "Halloween", "Fireball", "Teleport", "Burning", "Scorching", "Purple energy", "Green energy", "Nebula", "Purple stars", "Green stars", "Sunbeams", "Spellbound", "Purple sparks", "Yellow sparks", "Green zap", "Yellow zap", "Plasma", "Frostbite", "Time warp", "Purple souls", "Green souls", "Bubbles", "Hearts");
			CVarValues(ProjectileTrail, "След снаряда", std::string("Default"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				"Default", "None", "Rocket", "Critical", "Energy", "Charged", "Ray", "Fireball", "Teleport", "Fire", "Flame", "Sparks", "Flare", "Trail", "Health", "Smoke", "Bubbles", "Halloween", "Monoculus", "Sparkles", "Rainbow");
			CVarEnum(SpellFootsteps, "Следы заклинаний", 0, VISUAL, nullptr,
				VA_LIST("Выкл", "Цвет", "Свои", "Хеллоуин"),
				Off, Color, Team, Halloween);
			CVarEnum(RagdollEffects, "Эффекты рэгдолла", 0b000000, VISUAL | DROPDOWN_MULTI, nullptr,
				VA_LIST("Горение", "Электричество", "Пепел", "Растворение", "##Divider", "Золото", "Лёд"),
				Burning = 1 << 0, Electrocuted = 1 << 1, Ash = 1 << 2, Dissolve = 1 << 3, Gold = 1 << 4, Ice = 1 << 5);
			CVar(DrawIconsThroughWalls, "Иконки сквозь стены", false, VISUAL);
			CVar(DrawDamageNumbersThroughWalls, "Цифры урона сквозь стены", false, VISUAL);
		NAMESPACE_END(Tracers)

		NAMESPACE_BEGIN(Viewmodel)
			CVar(CrosshairAim, "Прицел при аиме", false, VISUAL);
			CVar(ViewmodelAim, "Модель оружия при аиме", false, VISUAL);
			CVar(OffsetX, VA_LIST("Смещение X", "Модель: смещение X"), 0.f, VISUAL | SLIDER_PRECISION, -45.f, 45.f, 5.f);
			CVar(OffsetY, VA_LIST("Смещение Y", "Модель: смещение Y"), 0.f, VISUAL | SLIDER_PRECISION, -45.f, 45.f, 5.f);
			CVar(OffsetZ, VA_LIST("Смещение Z", "Модель: смещение Z"), 0.f, VISUAL | SLIDER_PRECISION, -45.f, 45.f, 5.f);
			CVar(Pitch, VA_LIST("Питч", "Модель: питч"), 0.f, VISUAL | SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
			CVar(Yaw, VA_LIST("Йоу", "Модель: йоу"), 0.f, VISUAL | SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
			CVar(Roll, VA_LIST("Ролл", "Модель: ролл"), 0.f, VISUAL | SLIDER_CLAMP | SLIDER_PRECISION, -180.f, 180.f, 5.f);
			CVar(SwayScale, VA_LIST("Масштаб качания", "Модель: масштаб качания"), 0.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 5.f, 0.5f);
			CVar(SwayInterp, VA_LIST("Интерп. качания", "Модель: интерп. качания"), 0.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 1.f, 0.1f);
		NAMESPACE_END(Viewmodel)

		NAMESPACE_BEGIN(World)
			CVarEnum(Modulations, "Модуляции", 0b00000, VISUAL | DROPDOWN_MULTI, nullptr,
				VA_LIST("Мир", "Небо", "Объекты", "Частицы", "Туман"),
				World = 1 << 0, Sky = 1 << 1, Prop = 1 << 2, Particle = 1 << 3, Fog = 1 << 4);
			CVarValues(SkyboxChanger, "Смена скайбокса", std::string("Off"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				VA_LIST("Off"));
			CVarValues(WorldTexture, "Текстура мира", std::string("Default"), VISUAL | DROPDOWN_CUSTOM, nullptr,
				"Default", "Dev", "Camo", "Black", "White", "Gray", "Flat");
			CVar(NearPropFade, "Затухание ближних объектов", false, VISUAL);
			CVar(NoPropFade, "Без затухания объектов", false, VISUAL);
		NAMESPACE_END(World)

		NAMESPACE_BEGIN(Beams) // as of now, these will stay out of the menu
			CVar(Model, "Модель", std::string("sprites/physbeam.vmt"), VISUAL);
			CVar(Life, "Время жизни", 2.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
			CVar(Width, "Ширина", 2.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
			CVar(EndWidth, "Ширина конца", 2.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
			CVar(FadeLength, "Длина затухания", 10.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 30.f);
			CVar(Amplitude, "Амплитуда", 2.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
			CVar(Brightness, "Яркость", 255.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 255.f);
			CVar(Speed, "Скорость", 0.2f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 5.f);
			CVar(Segments, "Сегменты", 2, VISUAL | SLIDER_MIN, 1, 10);
			CVar(Color, "Цвет", Color_t(255, 255, 255, 255), VISUAL);
			CVarEnum(Flags, "Флаги", 0b10000000100000000, VISUAL | DROPDOWN_MULTI, nullptr,
				VA_LIST("Начальная сущность", "Конечная сущность", "Появление", "Исчезание", "Синус-шум", "Сплошной", "Затенение", "Рассеивание", "Шум один раз", "Без тайла", "По хитбоксам", "Начало видно", "Конец видно", "Активен", "Бесконечно", "Ореол", "Реверс"),
				StartEntity = 1 << 0, EndEntity = 1 << 1, FadeIn = 1 << 2, FadeOut = 1 << 3, SineNoise = 1 << 4, Solid = 1 << 5, ShadeIn = 1 << 6, ShadeOut = 1 << 7, OnlyNoiseOnce = 1 << 8, NoTile = 1 << 9, UseHitboxes = 1 << 10, StartVisible = 1 << 11, EndVisible = 1 << 12, IsActive = 1 << 13, Forever = 1 << 14, Halobeam = 1 << 15, Reverse = 1 << 16);
		NAMESPACE_END(Beams)

		NAMESPACE_BEGIN(Line)
			CVar(TracersEnabled, "Трейсеры вкл.", false, VISUAL);
			CVar(DrawDuration, VA_LIST("Длительность", "Линии: длительность"), 5.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
		NAMESPACE_END(Line)

		NAMESPACE_BEGIN(Hitbox)
			CVarEnum(BonesEnabled, VA_LIST("Кости вкл.", "Кости хитбоксов вкл."), 0b00, VISUAL | DROPDOWN_MULTI, "Выкл",
				VA_LIST("При выстреле", "При попадании"),
				OnShot = 1 << 0, OnHit = 1 << 1);
			CVarEnum(BoundsEnabled, VA_LIST("Границы вкл.", "Границы хитбоксов вкл."), 0b000, VISUAL | DROPDOWN_MULTI, "Выкл",
				VA_LIST("При выстреле", "При попадании", "Точка аима"),
				OnShot = 1 << 0, OnHit = 1 << 1, AimPoint = 1 << 2);
			CVar(DrawDuration, VA_LIST("Длительность", "Хитбоксы: длительность"), 5.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
		NAMESPACE_END(Hitbox)

		NAMESPACE_BEGIN(Prediction)
			CVarValues(PlayerPath, "Путь игрока", 0, VISUAL, nullptr,
				"Выкл", "Линия", "Разделители", "С промежутками", "Стрелки", "Боксы");
			CVarValues(ProjectilePath, "Путь снаряда", 0, VISUAL, nullptr,
				"Выкл", "Линия", "Разделители", "С промежутками", "Стрелки", "Боксы");
			CVar(SwingLines, "Линии удара", false, VISUAL);
			CVar(PlayerDrawDuration, VA_LIST("Длительность", "Путь игрока: длительность"), 5.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);
			CVar(ProjectileDrawDuration, VA_LIST("Длительность", "Путь снаряда: длительность"), 5.f, VISUAL | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f);

			CVarValues(RealPath, "Реальный путь", 0, NOSAVE | DEBUGVAR, nullptr,
				"Выкл", "Линия", "Разделители", "С промежутками", "Стрелки", "Боксы");
		NAMESPACE_END(Prediction)

		NAMESPACE_BEGIN(Simulation)
			CVarValues(TrajectoryPath, "Траектория", 0, VISUAL, nullptr,
				"Выкл", "Линия", "Разделители", "С промежутками", "Стрелки", "Боксы");
			CVarValues(ShotPath, "Путь выстрела", 0, VISUAL, nullptr,
				"Выкл", "Линия", "Разделители", "С промежутками", "Стрелки", "Боксы");
			CVarEnum(SplashRadius, "Радиус сплеша", 0b0, VISUAL | DROPDOWN_MULTI, "Выкл",
				VA_LIST("Ракеты", "Стики", "Пилюли", "Фейерверки", "##Divider", "Трейс", "Сфера"),
				Rockets = 1 << 0, Stickies = 1 << 1, Pipes = 1 << 2, Flares = 1 << 3, Trace = 1 << 4, Sphere = 1 << 5,
				Enabled = Rockets | Stickies | Pipes | Flares);
			CVar(ProjectileCamera, "Камера снаряда", false, VISUAL);
			CVar(ProjectileWindow, "Окно снаряда", WindowBox_t(), VISUAL | NOBIND);
			CVar(Box, VA_LIST("Бокс", "Бокс пути"), true, VISUAL);
		NAMESPACE_END(Simulation)

		NAMESPACE_BEGIN(Path)
			Enum(Style, Off, Line, Separators, Spaced, Arrows, Boxes);

			CVar(SeparatorSpacing, "Шаг разделителей", 4, NOSAVE | DEBUGVAR, 1, 16);
			CVar(SeparatorLength, "Длина разделителей", 12.f, NOSAVE | DEBUGVAR, 2.f, 16.f);
		NAMESPACE_END(Path)

		NAMESPACE_BEGIN(Trajectory)
			CVar(Override, "Переопределение симуляции", false, NOSAVE | DEBUGVAR);
			CVar(Type, "Тип", std::string("custom"), NOSAVE | DEBUGVAR);
			CVar(OffsetX, "Смещение X", 23.5f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -25.f, 25.f, 0.5f);
			CVar(OffsetY, "Смещение Y", 12.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -25.f, 25.f, 0.5f);
			CVar(OffsetZ, "Смещение Z", -3.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -25.f, 25.f, 0.5f);
			CVar(ForwardRedirect, "Перенаправление вперёд", 2000.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 2000.f, 100.f);
			CVar(ForwardCutoff, "Отсечение вперёд", 0.1f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 1.f, 0.1f);
			CVar(Hull, "Хулл", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f, 0.5f);
			CVar(Speed, "Скорость", 1100.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 5000.f, 50.f);
			CVar(Gravity, "Гравитация", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 800.f, 100.f);
			CVar(LifeTime, "Время жизни", 10.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 10.f, 0.1f);
			CVar(UpVelocity, "Скорость вверх", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 1000.f, 50.f);
			CVar(AngularVelocityX, "Угл. скорость X", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -1000.f, 1000.f, 50.f);
			CVar(AngularVelocityY, "Угл. скорость Y", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -1000.f, 1000.f, 50.f);
			CVar(AngularVelocityZ, "Угл. скорость Z", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, -1000.f, 1000.f, 50.f);
			CVar(Drag, "Сопротивление", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 2.f, 0.1f);
			CVar(DragX, "Сопротивление X", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 0.1f, 0.01f, "%.15g");
			CVar(DragY, "Сопротивление Y", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 0.1f, 0.01f, "%.15g");
			CVar(DragZ, "Сопротивление Z", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 0.1f, 0.01f, "%.15g");
			CVar(AngularDragX, "Угл. сопротивление X", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 0.1f, 0.01f, "%.15g");
			CVar(AngularDragY, "Угл. сопротивление Y", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 0.1f, 0.01f, "%.15g");
			CVar(AngularDragZ, "Угл. сопротивление Z", 0.f, NOSAVE | DEBUGVAR | SLIDER_PRECISION, 0.f, 0.1f, 0.01f, "%.15g");
			CVar(MaxVelocity, "Макс. скорость", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 4000.f, 50.f);
			CVar(MaxAngularVelocity, "Макс. угл. скорость", 0.f, NOSAVE | DEBUGVAR | SLIDER_MIN | SLIDER_PRECISION, 0.f, 7200.f, 50.f);
		NAMESPACE_END(ProjectileTrajectory)
	NAMESPACE_END(Visuals)

	NAMESPACE_BEGIN(Misc)
		NAMESPACE_BEGIN(Movement)
			CVarEnum(AutoStrafe, "Автострейф", 0, NONE, nullptr,
				VA_LIST("Выкл", "Легитный", "Направленный"),
				Off, Legit, Directional);
			CVar(AutoStrafeTurnScale, VA_LIST("Масштаб поворота", "Автострейф: масштаб поворота"), 0.5f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 1.f, 0.1f);
			CVar(AutoStrafeMaxDelta, VA_LIST("Макс. дельта", "Автострейф: макс. дельта"), 180.f, SLIDER_CLAMP | SLIDER_PRECISION, 0.f, 180.f, 5.f);
			CVar(Bunnyhop, "Баннихоп", false);
			CVar(EdgeJump, "Прыжок с края", false);
			CVar(AutoJumpbug, "Автоджампбаг", false);
			CVar(BreakJump, "Брейк-джамп", false);
			CVar(AutoRocketJump, "Авто рокет-джамп", false);
			CVar(AutoCTap, "Авто c-tap", false);
			CVar(AutoFaNJump, "Авто прыжок FaN", false);
			CVar(AutoRevJump, "Авто прыжок от рева", false);
			CVar(FastStop, "Быстрая остановка", false);
			CVar(FastAccelerate, "Быстрое ускорение", false);
			CVar(DuckSpeed, "Скорость приседания", false);
			CVar(ShieldTurnRate, "Скорость поворота щита", false);
			CVar(NoPush, "Без отталкивания", false);
			CVar(MovementLock, "Блокировка движения", false);

			CVar(AutoRocketJumpChokeGrounded, "Чоук на земле", 1, NOSAVE | DEBUGVAR, 0, 3);
			CVar(AutoRocketJumpChokeAir, "Чоук в воздухе", 1, NOSAVE | DEBUGVAR, 0, 3);
			CVar(AutoRocketJumpSkipGround, "Пропуск на земле", 0, NOSAVE | DEBUGVAR, 0, 3);
			CVar(AutoRocketJumpSkipAir, "Пропуск в воздухе", 1, NOSAVE | DEBUGVAR, 0, 3);
			CVar(AutoRocketJumpTimingOffset, "Смещение тайминга", 0, NOSAVE | DEBUGVAR, 0, 3);
			CVar(AutoRocketJumpApplyAbove, "Применять смещение выше", 0, NOSAVE | DEBUGVAR, 0, 10);
		NAMESPACE_END(Movement)

		NAMESPACE_BEGIN(Automation)
			CVarEnum(AntiBackstab, "Анти-бэкстаб", 0, NONE, nullptr,
				VA_LIST("Выкл", "Йоу", "Питч", "Фейк"),
				Off, Yaw, Pitch, Fake);
			CVar(TauntControl, "Контроль таунтов", false);
			CVar(KartControl, "Контроль карта", false);
			CVar(AntiAutobalance, "Анти-автобаланс", false);
			CVar(AntiAFK, "Анти-AFK", false);
			CVar(AutoF2Ignored, "Авто F2 игнор", false);
			CVar(AutoF1Priority, "Авто F1 приоритет", false);
			CVar(AcceptItemDrops, "Авто приём дропа", false);
		NAMESPACE_END(Automation)

		NAMESPACE_BEGIN(Exploits)
			CVar(PureBypass, "Обход pure", false);
			CVar(CheatsBypass, "Обход sv_cheats", false);
			CVar(UnlockCVars, "Разблокировать CVars", false);
			CVar(EquipRegionUnlock, "Разблокировать слоты", false);
			CVar(BackpackExpander, "Расширитель рюкзака", false);
			CVar(NoisemakerSpam, "Спам шумогенератором", false);
			CVar(PingReducer, "Снижение пинга", false);
			CVar(PingTarget, "cl_cmdrate", 1, SLIDER_CLAMP, 1, 66);
		NAMESPACE_END(Exploits)

		NAMESPACE_BEGIN(Game)
			CVar(NetworkFix, "Фикс сети", false);
			CVar(SetupBonesOptimization, "Оптимизация костей", false);
			CVar(AntiCheatCompatibility, "Совместимость с анти-читом", false);

			CVar(AntiCheatCritHack, "Критхак с анти-читом", false, NOSAVE | DEBUGVAR);
		NAMESPACE_END(Game)

		NAMESPACE_BEGIN(Queueing)
			CVarEnum(ForceRegions, "Форс регионов", 0b0, DROPDOWN_MULTI, nullptr, // i'm not sure all of these are actually used for tf2 servers
				VA_LIST("Atlanta", "Chicago", "Dallas", "Los Angeles", "Seattle", "Virginia", "##Divider", "Amsterdam", "Falkenstein", "Frankfurt", "Helsinki", "London", "Madrid", "Paris", "Stockholm", "Vienna", "Warsaw", "##Divider", "Buenos Aires", "Lima", "Santiago", "Sao Paulo", "##Divider", "Chennai", "Dubai", "Hong Kong", "Mumbai", "Seoul", "Singapore", "Tokyo", "##Divider", "Sydney", "##Divider", "Johannesburg"),
				// North America
				ATL = 1 << 0, // Atlanta
				ORD = 1 << 1, // Chicago
				DFW = 1 << 2, // Dallas
				LAX = 1 << 3, // Los Angeles
				SEA = 1 << 4, // Seattle (+DC_EAT?)
				IAD = 1 << 5, // Virginia
				// Europe
				AMS = 1 << 6, // Amsterdam
				FSN = 1 << 7, // Falkenstein
				FRA = 1 << 8, // Frankfurt
				HEL = 1 << 9, // Helsinki
				LHR = 1 << 10, // London
				MAD = 1 << 11, // Madrid
				PAR = 1 << 12, // Paris
				STO = 1 << 13, // Stockholm
				VIE = 1 << 14, // Vienna
				WAW = 1 << 15, // Warsaw
				// South America
				EZE = 1 << 16, // Buenos Aires
				LIM = 1 << 17, // Lima
				SCL = 1 << 18, // Santiago
				GRU = 1 << 19, // Sao Paulo
				// Asia
				MAA = 1 << 20, // Chennai
				DXB = 1 << 21, // Dubai
				HKG = 1 << 22, // Hong Kong
				BOM = 1 << 23, // Mumbai
				SEO = 1 << 24, // Seoul
				SGP = 1 << 25, // Singapore
				TYO = 1 << 26, // Tokyo
				// Australia
				SYD = 1 << 27, // Sydney
				// Africa
				JNB = 1 << 28, // Johannesburg
			);
			CVar(ExtendQueue, "Расширить очередь", false);
			CVar(AutoCasualQueue, "Авто очередь Casual", false);
		NAMESPACE_END(Queueing)

		NAMESPACE_BEGIN(MannVsMachine, Mann vs. Machine)
			CVar(InstantRespawn, "Мгновенный респавн", false);
			CVar(InstantRevive, "Мгновенное воскрешение", false);
			CVar(AllowInspect, "Разрешить осмотр", false);
		NAMESPACE_END(Sound)

		NAMESPACE_BEGIN(Sound)
			CVarEnum(Block, VA_LIST("Блок", "Блок звуков"), 0b0000, DROPDOWN_MULTI, nullptr,
				VA_LIST("Шаги", "Шумогенератор", "Сковорода", "Вода"),
				Footsteps = 1 << 0, Noisemaker = 1 << 1, FryingPan = 1 << 2, Water = 1 << 3);
			CVar(HitsoundAlways, "Хитсаунд всегда", false);
			CVar(RemoveDSP, "Убрать DSP", false);
			CVar(GiantWeaponSounds, "Звуки оружия гигантов", false);
		NAMESPACE_END(Sound)
	NAMESPACE_END(Misc)

	NAMESPACE_BEGIN(Logging)
		CVarEnum(Logs, "Логи", 0b0000011, DROPDOWN_MULTI, "Выкл",
			VA_LIST("Начало голосования", "Голос отдан", "Смена класса", "Урон", "Детект читеров", "Tags", "Алиасы", "Резолвер"),
			VoteStart = 1 << 0, VoteCast = 1 << 1, ClassChanges = 1 << 2, Damage = 1 << 3, CheatDetection = 1 << 4, Tags = 1 << 5, Aliases = 1 << 6, Resolver = 1 << 7);
		Enum(LogTo, Toasts = 1 << 0, Chat = 1 << 1, Party = 1 << 2, Console = 1 << 3, Menu = 1 << 4, Debug = 1 << 5);
		CVarEnum(NotificationPosition, "Позиция уведомлений", 0, VISUAL, nullptr,
			VA_LIST("Сверху слева", "Сверху справа", "Снизу слева", "Снизу справа"),
			TopLeft, TopRight, BottomLeft, BottomRight);
		CVar(NotificationTime, "Время уведомления", 5.f, VISUAL, 0.5f, 5.f, 0.5f);
		CVar(MaxNotifications, "Макс. уведомлений", 10, VISUAL | SLIDER_MIN, 1, 10);

		NAMESPACE_BEGIN(VoteStart, Logging)
			CVarValues(LogTo, "Голосование начато: куда", 0b000001, DROPDOWN_MULTI, nullptr,
				"Тосты", "Чат", "Патька", "Консоль", "Меню", "Отладка");
		NAMESPACE_END(VoteStart)

		NAMESPACE_BEGIN(VoteCast, Logging)
			CVarValues(LogTo, "Голос отдан: куда", 0b000001, DROPDOWN_MULTI, nullptr,
				"Тосты", "Чат", "Патька", "Консоль", "Меню", "Отладка");
		NAMESPACE_END(VoteCast)

		NAMESPACE_BEGIN(ClassChange, Logging)
			CVarValues(LogTo, "Смена класса: куда", 0b000001, DROPDOWN_MULTI, nullptr,
				"Тосты", "Чат", "Патька", "Консоль", "Меню", "Отладка");
		NAMESPACE_END(ClassChange)

		NAMESPACE_BEGIN(Damage, Logging)
			CVarValues(LogTo, "Урон: куда", 0b000001, DROPDOWN_MULTI, nullptr,
				"Тосты", "Чат", "Патька", "Консоль", "Меню", "Отладка");
		NAMESPACE_END(Damage)

		NAMESPACE_BEGIN(CheatDetection, Logging)
			CVarValues(LogTo, "Детект читеров: куда", 0b000001, DROPDOWN_MULTI, nullptr,
				"Тосты", "Чат", "Патька", "Консоль", "Меню", "Отладка");
		NAMESPACE_END(CheatDetection)

		NAMESPACE_BEGIN(Tags, Logging)
			CVarValues(LogTo, "Теги: куда", 0b000001, DROPDOWN_MULTI, nullptr,
				"Тосты", "Чат", "Патька", "Консоль", "Меню", "Отладка");
		NAMESPACE_END(Tags)

		NAMESPACE_BEGIN(Aliases, Logging)
			CVarValues(LogTo, "Алиасы: куда", 0b000001, DROPDOWN_MULTI, nullptr,
				"Тосты", "Чат", "Патька", "Консоль", "Меню", "Отладка");
		NAMESPACE_END(Aliases)

		NAMESPACE_BEGIN(Resolver, Logging)
			CVarValues(LogTo, "Резолвер: куда", 0b000001, DROPDOWN_MULTI, nullptr,
				"Тосты", "Чат", "Патька", "Консоль", "Меню", "Отладка");
		NAMESPACE_END(Resolver)
	NAMESPACE_END(Logging)

	NAMESPACE_BEGIN(CheatDetection, Cheat Detection)
		CVarEnum(Methods, "Методы детекта", 0b0000, DROPDOWN_MULTI, nullptr,
			VA_LIST("Неверный питч", "Чокинг пакетов", "Флики аима", "Скорость приседа"),
			InvalidPitch = 1 << 0, PacketChoking = 1 << 1, AimFlicking = 1 << 2, DuckSpeed = 1 << 3);
		CVar(DetectionsRequired, "Нужно детектов", 10, SLIDER_MIN, 0, 50);
		CVar(MinChoking, "Мин. чоук", 20, SLIDER_MIN, 4, 22);
		CVar(MinFlick, "Мин. угол флика", 20.f, SLIDER_PRECISION, 10.f, 30.f); // min flick size to suspect
		CVar(MaxNoise, "Макс. шум флика", 1.f, SLIDER_PRECISION, 1.f, 10.f); // max difference between angles before and after flick
	NAMESPACE_END(CheatDetection)

	NAMESPACE_BEGIN(Debug)
		CVar(Info, "Отладочная информация", false, NOSAVE);
		CVar(Logging, "Отладочные логи", false, NOSAVE);
		CVar(Options, "Отладочные опции", false, NOSAVE);
		CVar(CrashLogging, "Логи крашей", true, NOBIND);

#ifdef DEBUG_TRACES
		CVar(VisualizeTraces, "Визуализация трейсов", false, NOSAVE);
		CVar(VisualizeTraceHits, "Визуализация попаданий", false, NOSAVE);
#endif

		CVar(DrawHitboxes, "Показать хитбоксы", false, NOSAVE);
	NAMESPACE_END(Debug)
NAMESPACE_END(Vars)
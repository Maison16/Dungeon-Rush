/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lcd.h"
#include "math.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
RTC_TimeTypeDef sTime;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_RTC_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char czas[16];
// do naszej plytki
//const float napiecieSelect=3.24;
//const float napieciePrawo=0;
//const float napiecieLewo=2.23;
//const float napiecieDol=1.46;
//const float napiecieGora=0.64;
//NAPI�?CIE DOMYŚLNE 2.82-2.83
// do plytki w sali
const float napiecieSelect=2.27;
const float napieciePrawo=0;
const float napiecieLewo=1.71;
const float napiecieDol=1.22;
const float napiecieGora=0.59;
int odwiedzone[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // sluzy do oznaczania czy pokoj byl odwiedzony
char aktualny[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' '}; // sluzy do oznaczania aktualnego pokoju
int zycie=100; // gracza
int skillMieczUzycia=3;
int skillTarczaUzycia=3;
int skillCzarUzycia=3;
int ogranicznik = 1;
int dmgLosowy=0;
int dmgGracza=5;
int koniec = 1;
int klucze = 0;
const uint8_t zycieL[] =
    {
     0b00110,
     0b01001,
     0b10000,
     0b10000,
     0b01000,
     0b00100,
     0b00010,
     0b00001
    };
const uint8_t zycieP[] =
    {
  	0b01100,
	0b10010,
  	0b00001,
  	0b00001,
    0b00010,
  	0b00100,
	0b01000,
  	0b10000
    };
const uint8_t skillMiecz[] =
	  {
		0b00100,
		0b00100,
		0b00100,
		0b00100,
		0b00100,
		0b01110,
		0b00100,
		0b00100
	  };
const uint8_t skillTarcza[] =
	  {
		0b00000,
		0b10101,
		0b11111,
		0b11111,
		0b11111,
		0b01110,
		0b00100,
		0b00000
	  };
const uint8_t skillCzar[] =
	  {
		0b00000,
		0b10101,
		0b01110,
		0b11111,
		0b01110,
		0b10101,
		0b00000,
		0b00000
	  };
const uint8_t czasIkonka[] =
	  {
		0b00000,
		0b11011,
		0b11111,
		0b10001,
		0b10101,
		0b10001,
		0b11111,
		0b00000
	  };
void lcd_wlasny_znak(uint8_t adres, const char* znak)
{
         adres &= 0x07;
         lcd_cmd(0x40 + (adres * 8));
         for (int i = 0; i < 8; ++i)
         {
             lcd_char_cp(znak[i]);
         }
}

float napiecie_drabiny()
{
	HAL_ADC_Start(&hadc1); //zaczyna pomiar
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	HAL_ADC_Stop(&hadc1); //konczy pomiar
	uint16_t pomiar = HAL_ADC_GetValue(&hadc1);
	float napiecie = (float)pomiar * 3.3 / (pow(2,12)-1); //n oznacza rozdzielczosc i dla nas wynosi 12
	return napiecie;
}
int uzycieSkilla(float napiecie)
{
      if(napiecie<napiecieGora+0.02 && napiecie>napiecieGora-0.02)
            {
          if(skillMieczUzycia!=0)
          {
                skillMieczUzycia--;
                return 2;
          }
          else{
        	  printf("\n\rTwoj miecz natychmiastowo gasnie.\n\r");
        	  printf("Nie mozesz uzyc tej umiejetnosci\n\r");
        	  return 10;
          }
            }
      else if(napiecie<napiecieLewo+0.02 && napiecie>napiecieLewo-0.02)
           {
          if(skillTarczaUzycia!=0)
          {
                skillTarczaUzycia--;
                return 1;
          }
          else{
        	  printf("\n\rTwoja tarcza nie nadaje sie do uzytku.\n\r");
              printf("Nie mozesz uzyc tej umiejetnosci\n\r");
              return 10;
                    }
           }
      else if(napiecie<napieciePrawo+0.02 && napiecie>napieciePrawo-0.02)
           {
          if(skillCzarUzycia!=0)
          {
                skillCzarUzycia--;
                return 3;
          }
          else{
        	  printf("\n\rCzar rozprasza ci sie w dloni.\n\r");
              printf("Nie mozesz uzyc tej umiejetnosci\n\r");
              return 10;
                    }
           }
          else if(napiecie<napiecieDol+0.02 && napiecie>napiecieDol-0.02)
        	  return 4;
      else
    	  return 10;
}
int __io_putchar(int ch)
{
    if (ch == '\n') {
        uint8_t ch2 = '\r';
        HAL_UART_Transmit(&huart2, &ch2, 1, HAL_MAX_DELAY);
    }

    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return 1;
}
int _write(int file, char *ptr, int len)
{
for (int i = 0; i < len; i++) {
__io_putchar(ptr[i]);
}
return len;
}
void wyswietlInterfejs()
{
	  char buffer[16];
	  char buffer2[16];
	  char buffer3[16];
	  char buffer4[16];
	  char buffer5[16];

	  lcd_wlasny_znak(0, zycieL);
	  lcd_wlasny_znak(1, zycieP);
	  lcd_wlasny_znak(2, skillTarcza);
	  lcd_wlasny_znak(3, skillMiecz);
	  lcd_wlasny_znak(4, skillCzar);
	  lcd_wlasny_znak(5, czasIkonka);
	  sprintf(buffer,"%d", zycie);
	  sprintf(buffer2,"<%d", skillTarczaUzycia);
	  sprintf(buffer3,"^%d", skillMieczUzycia);
	  sprintf(buffer4,">%d", skillCzarUzycia);
	  sprintf(buffer5,"BUTTON");
	  lcd_char(1, 1, 0);
	  lcd_char(1, 2, 1);
	  lcd_char(2, 1, 2);
	  lcd_char(2, 7, 3);
	  lcd_char(2, 14, 4);
	  lcd_char(1, 10, 5);
	  lcd_print(1,3,buffer);
	  lcd_print(2,2,buffer2);
	  lcd_print(2,8,buffer3);
	  lcd_print(2,15,buffer4);
	  lcd_print(1,11,buffer5);
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	 if (GPIO_Pin == BT1_Pin)
	    {
		 if(ogranicznik == 1)
		 {
		 printf("\r\n");
		 printf("\r\n");
		 printf("    MENU");
		 printf("\r\n");
		 printf("\r\n");
		 printf("Zdobyte klucze %d/2", klucze);
		 printf("\r\n");
		 printf("\r\n");
		 printf("-----------------\r\n");
		 printf("|czas gry: %s|\r\n", &czas);
		 printf("-----------------\r\n");
		 printf("\r\n");
		 printf("Twoje polozenie:");
		 printf("\r\n");
		 printf(" ________________ \r\n");
		 printf("|          _    |\r\n");
		 printf("|      _  |%c|   |\r\n", aktualny[10]);
		 printf("|  _ _|%c|_|%c|_  |\r\n", aktualny[4],aktualny[9]);
		 printf("| |B|%c|%c|%c|%c|%c| |\r\n", aktualny[2], aktualny[3], aktualny[0], aktualny[6], aktualny[11]);
		 printf("|       |%c|%c|   |\r\n", aktualny[5],aktualny[7]);
		 printf("|         |%c|   |\r\n", aktualny[8]);
		 printf("|               |\r\n");
		 printf("|_______________|\r\n");
		 printf("\r\n");
		 printf("\r\nDostepne umiejetnosci:\r\n"
								" LEFT - Obrona: Blokuje obrazenia przez 2 tury\r\n"
								" RIGHT - Czar: Zadaje obrazenia zwyklego ataku oraz wzmacnia nastepne 2 ataki\r\n"
								" UP - Wzmocniony atak: Zadaje potrojone obrazenia\r\n"
								" DOWN - Podstawowy atak: Zadaje normalne obrazenia\r\n\r\n");
		 ogranicznik=0;
		 }
		 else
		 {
		 ogranicznik=1;
		 }
	    }
}
void init_RTC(void) {

  sTime.Hours = 0;
  sTime.Minutes = 0;
  sTime.Seconds = 0;


  HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);


}
int walka(int hpOponenta, int dmgOponenta)
{
	int licznikTarczy = 0; // na ile tur jest tarcza
	int licznikCzaru = 0;  // na ile tur jest czar
	srand(time(0));
	float napiecie;
while(hpOponenta>0 && zycie >0)
{
	int turagracza=10;
	while(turagracza!=0)
	{
		if(ogranicznik)
		{
		printf("\r\nTWOJA TURA :\r\n");
		ogranicznik=0;
		}
		napiecie = napiecie_drabiny();
		while (napiecie > napiecieSelect){
			napiecie = napiecie_drabiny();
			turagracza=uzycieSkilla(napiecie);
		}
		dmgLosowy = 1 + rand() % 6;
		if(turagracza == 1)
		{
			///// logika zadawania dmg przez skilla
			licznikTarczy += 2;
			printf("\r\nZaslaniasz sie swoja tarcza (niewrazliwosc na 2 tury)\r\n");
			turagracza = 0;
		}

		else if(turagracza == 2)
		{
			if(licznikCzaru > 0)
			{
				licznikCzaru--;
				hpOponenta-= (3 * dmgGracza + dmgLosowy + 4);
				printf("\r\nZadano: %d obrazen\r\n", 3 * dmgGracza+dmgLosowy+4);
				printf("Pozostale zdrowie przeciwnika: %d\r\n", hpOponenta);
				turagracza = 0;
			}
			else
			{
				hpOponenta-= (3 * dmgGracza + dmgLosowy);
				printf("\r\nZadano: %d obrazen\r\n", 3 * dmgGracza+dmgLosowy);
				printf("Pozostale zdrowie przeciwnika: %d\r\n", hpOponenta);
				turagracza = 0;
			}
		}

		else if(turagracza == 3)
		{
			if(licznikCzaru > 0)
				{
					licznikCzaru--;
					hpOponenta-= (dmgGracza + dmgLosowy + 4);
					printf("\r\nZadano: %d obrazen\r\n", dmgGracza+dmgLosowy+4);
					printf("Pozostale zdrowie przeciwnika: %d\r\n", hpOponenta);
				}
			else
				{
					hpOponenta-= (dmgGracza + dmgLosowy);
					printf("\r\nZadano: %d obrazen\r\n", dmgGracza+dmgLosowy);
					printf("Pozostale zdrowie przeciwnika: %d\r\n", hpOponenta);
				}
			licznikCzaru += 2;
			printf("\r\nUzyto czaru \r\n");
			turagracza = 0;
		}

		else if(turagracza == 4)
		{
			if(licznikCzaru > 0)
			{
				licznikCzaru--;
				hpOponenta-= (dmgGracza + dmgLosowy + 4);
				printf("\r\nZadano: %d obrazen\r\n", dmgGracza+dmgLosowy+4);
				printf("Pozostale zdrowie przeciwnika: %d\r\n", hpOponenta);
				turagracza = 0;
			}
			else
			{
				hpOponenta-= (dmgGracza + dmgLosowy);
				printf("\r\nZadano: %d obrazen\r\n", dmgGracza+dmgLosowy);
				printf("Pozostale zdrowie przeciwnika: %d\r\n", hpOponenta);
				turagracza = 0;
			}

		}
	}
	ogranicznik=1;
	HAL_Delay(250);
	//ruch przeciwnika
	if(hpOponenta > 0)
	{
		if(licznikTarczy > 0)
		{
			licznikTarczy--;
			printf("\r\nPrzeciwnik nie zadal obrazen\r\n");
		}
		else
		{
			lcd_clear();
			dmgLosowy = 1 + rand() % 6;
			zycie-= (dmgOponenta + dmgLosowy);
			printf("\r\nPrzeciwnik zadal: %d obrazen\r\n", dmgOponenta+dmgLosowy);
		}
	}
	wyswietlInterfejs();
	HAL_Delay(250);
}
if(zycie<=0)
	{
	printf("\r\nBohater umiera!\r\n");
	return 0;
	}
else if(hpOponenta<=0)
	{
	printf("\r\nPrzeciwnik pokonany\r\n");
	return 1;
	}
}
void sciana() // ogolnie napisane tylko po to bo nie chce mi sie tego kopiowac za kazdym razem i to nieprofesojonalne
{
	  printf("\r\n___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n\r\n");
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART2_UART_Init();
  MX_RTC_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  lcd_init(_LCD_4BIT, _LCD_FONT_5x8, _LCD_2LINE);
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);
   float napiecie;
   char buffer[16];
   uint8_t otworzyc; //skrzynie
   int wybor = 0; // zmienna do okreslania polozenia danego pokoju
   uint8_t gdzieidzie; // pomocna zmienna do okreslenia wyboru uzytkownika
   init_RTC();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	  printf("Nazywasz sie Dorian Lightbringer, utalentowany paladyn swiatla.\r\n");
	  printf("Podobnie jak wielu innych smialkow przed Toba udaje ci sie dotrzec do lochu.\r\n");
	  printf("Okazuje to sie to jednak smiertelna pulapka.\r\n");
	  printf("Nie mozesz opuscic lochu dopoki nie pokonasz Najwyzszego Straznika.\r\n");
	  printf("Co gorsza czujesz ze mrok lochu zaczyna cie pochlaniac.\r\n");
	  printf("~~PALADYNIE POSPIESZ SIE DOPOKI NIE JEST ZA POZNO~~.\r\n");
	  printf("\r\n(Poruszanie odbywa sie przy pomocy cyfr, a walka przy pomocy przyciskow tak samo jak menu)\r\n");
	  printf("     _,.\r\n");
	  printf("    ,` -.)\r\n");
	  printf("   ( _/-\\-._\r\n");
	  printf("  /,|`--._,-^|            ,\r\n");
	  printf("  \\_| |`-._/||          ,'| \r\n");
	  printf("    |  `-, / |         /  / \r\n");
	  printf("    |     || |        /  / \r\n");
	  printf("     `r-._||/   __   /  / \r\n");
	  printf(" __,-<_     )`-/  `./  / \r\n");
	  printf("'  \\   `---'   \\   /  / \r\n");
	  printf("    |           |./  / \r\n");
	  printf("    /           //  / \r\n");
	  printf("\\_/' \\         |/  / \r\n");
	  printf(" |    |   _,^-'/  / \r\n");
	  printf(" |    , ``  (\\/  /_\r\n");
	  printf("  \\,.->._    \\X-=/^\r\n");
	  printf("  (  /   `-._//^`\r\n");
	  printf("   `Y-.____(__}\r\n");
	  printf("    |     {__)\r\n");
	  printf("          ()\r\n");
	  while ((zycie > 0) && (koniec))
	  {
	      napiecie = napiecie_drabiny();
	      uzycieSkilla(napiecie);
	      wyswietlInterfejs();

	      // Czekamy, aż bufor UART zostanie w pełni odblokowany
	      while (HAL_UART_GetState(&huart2) == HAL_UART_STATE_BUSY_RX)
	      {
	          // Czekamy na zakończenie transmisji
	      }

	      // poczatek gry
			  switch (wybor)
			  {
			  case 0: // pokoj startowy
			  pokojzero:
			  aktualny[wybor]='X';
				  if (!odwiedzone[wybor])
				  {
					  printf("\r\nA wiec wchodzisz do lochu przez strawione przez zab czasu drzwi. Widzisz, ze "
							 "pokoj w ktorym sie znalazles posiada trzy rozwidlenia.\r\n");
					  odwiedzone[wybor] = 1;
				  }
				  printf("\r\nGdzie sie udajesz ?\r\n");
				  printf("4. lewo\r\n");
				  printf("6. prawo\r\n");
				  printf("2. dol\r\n");
			      HAL_UART_Receive(&huart2, &gdzieidzie, 1, HAL_MAX_DELAY);
			      aktualny[wybor]=' ';
				  if (gdzieidzie == '4'){
					  wybor = 3;
					  if (odwiedzone[3])
						  printf("\r\nOdwiedziles juz ten pokoj. Cialo rozplatanego pajaka lezy tak samo jak wczesniej\r\n");
				  }
				  else if (gdzieidzie == '6'){
					  wybor = 6;
					  if (odwiedzone[6])
						  printf("\r\nTutaj pokonales szkieleta na co wskazuje polamany szkielet lezacy na podlodze\r\n");
				  }
				  else if (gdzieidzie == '2'){
					  wybor = 5;
					  if (odwiedzone[5])
						  printf("\r\nOdwiedziles juz ten pokoj, resztki nieumarlego wojownika rozrzucone sa po pomieszczeniu\r\n");
				  }
				  else if (gdzieidzie == '8'){
					  printf("\r\n|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|_____|_____|_____|______|____|_____|_____|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____|_____|_____|,-' ;  ! `-._____|_____|_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|_____|_____|__/ :  !  :  . \_|_____|_____|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____|_____|____|_ ;   __:  ;  |___|_____|_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|_____|_____|_)| .  :)(.  !  ||_____|_____|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____|_____|____|\"    (##)  _  |___|_____|_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|_____|_____|_|  :  ;`'  (_) (|_____|_____|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____|_____|____|  :  :  .     |___|_____|_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|_____|_____|_)_ !  ,  ;  ;  ||_____|_____|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____|_____|____|| .  .  :  :  |___|_____|_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|_____|_____|_|\" .  |  :  .  ||_____|_____|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____|_____|____|____;----.____|___|_____|_____|_____|_____|_____|_____|__\r\n\r\n");
					  printf("\r\nStad przyszedles ale drzwi zatrzasnely sie po twoim wejsciu musisz poszukac innego wyjscia !\r\n");
					  goto pokojzero;
				  }
				  break;
			  case 1: // pokoj z bossem - Najwyzszy Straznik
			  pokojjeden:
			  aktualny[wybor]='X';
					    printf("                                         .\"--..__\r\n");
					    printf("                     _                     []       ``-.._\r\n");
					    printf("                  .'` `'.                  ||__           `-._\r\n");
					    printf("                 /    ,-\\                 ||_ ```---..__     `-.\r\n");
					    printf("                /    /::\\\\               /|//}          ``--._  `.\r\n");
					    printf("                |    |:::||              |////}                `-. \\\r\n");
					    printf("                |    |:::||             //'///                    `.\\\r\n");
					    printf("                |    |:::||            //  ||'                      `|\r\n");
					    printf("                /    |:::|/        _,-//\\  ||\r\n");
					    printf("               /`    |:::|`-,__,-'`  |/  \\ ||\r\n");
					    printf("             /`  |   |'' ||           \\   |||\r\n");
					    printf("           /`    \\   |   ||            |  /||\r\n");
					    printf("         |`       |  |   |)            \\ | ||\r\n");
					    printf("        |          \\ |   /      ,.__    \\| ||\r\n");
					    printf("        /           `         /`    `\\   | ||\r\n");
					    printf("       |                     /        \\  / ||\r\n");
					    printf("       |                     |        | /  ||\r\n");
					    printf("       /         /           |        `(   ||\r\n");
					    printf("      /          .           /          )  ||\r\n");
					    printf("     |            \\          |     ________||\r\n");
					    printf("    /             |          /     `-------.|\r\n");
					    printf("   |\\            /          |              ||\r\n");
					    printf("   \\/`-._       |           /              ||\r\n");
					    printf("    //   `.    /`           |              ||\r\n");
					    printf("   //`.    `. |             \\              ||\r\n");
					    printf("  ///\\ `-._  )/             |              ||\r\n");
					    printf(" //// )   .(/               |              ||\r\n");
					    printf(" ||||   ,'` )               /              //\r\n");
					    printf(" ||||  /                    /             || \r\n");
					    printf(" `\\` /`                    |             // \r\n");
					    printf("     |`                     \\            ||  \r\n");
					    printf("    /                        |           //  \r\n");
					    printf("  /`                          \\         //   \r\n");
					    printf("/`                            |        ||    \r\n");
					    printf("`-.___,-.      .-.        ___,'        (/    \r\n");
					    printf("         `---'`   `'----'`\r\n");

					  printf("\r\nPrzekrecasz dwa klucze zdobyte w lochu poczym widzisz Najwyzszego Straznika \r\n");
					  printf("To od niego bije przerazajacy mrok, wiesz ze zostalo ci malo czasu musisz sie z nim szybko uporac !\r\n");
					  if(walka(85, 9))
					  {
					  odwiedzone[wybor] = 1;
					  koniec = 0;
					  break;
					  }
					  else
					  {
						  koniec = 0;
						  break;

					  }
			  case 2: // pokoj szkielet potka
			  pokojdwa:
			  aktualny[wybor]='X';
				  if (!odwiedzone[wybor])
				  {
					    odwiedzone[wybor] = 1;
					    printf("                              _.--\"\"-._\n\r");
					    printf("  .                         .\"         \".\n\r");
					    printf(" / \\    ,^.         /(     Y             |      )\\\n\r");
					    printf("/   `---. |--'\\    (  \\__..'--   -   -- -'\"\"-.-'  )\n\r");
					    printf("|        :|    `>   '.     l_..-------.._l      .'\n\r");
					    printf("|      __l;__ .'      \"-.__.||_.-'v'-._||`\"----\"\n\r");
					    printf("\\  .-' | |  `              l._       _.'\n\r");
					    printf(" \\/    | |                   l`^^'^^'j\n\r");
					    printf("       | |                _   \\_____/     _\n\r");
					    printf("       | |               l `--__)-'(__.--' |\n\r");
					    printf("       | |               | /`---``-----'\"1 |  ,-----.\n\r");
					    printf("       | |               )/  `--' '---'   \\'-'  ___  `-. \n\r");
					    printf("     _ L |_            //  `-'  '`----'  /  /  |   |  `. \\\n\r");
					    printf("    '._' / \\         _/(   `/   )- ---' ;  /__.J   L.__.\\ :\n\r");
					    printf("      `._;/7(-.......'  /        ) (     |  |            | |\n\r");
					    printf("      `._;l _'--------_/        )-'/     :  |___.    _._./ ;\n\r");
					    printf("        | |                 .__ )-'\\  __  \\  \\  I   1   / /\n\r");
					    printf("        `-'                /   `-\\-(-'   \\ \\  `.|   | ,' /\n\r");
					    printf("                           \\__  `-'    __/  `-. `---'',-'\n\r");
					    printf("                              )-._.-- (        `-----'\n\r");
					    printf("                             )(  l\\ o ('..-.\n\r");
					    printf("                       _..--' _'-' '--'.-. |\n\r");
					    printf("                __,,-'' _,,-''            \\ \\\n\r");
					    printf("               f'. _,,-'                   \\ \\\n\r");
					    printf("              ()--  |                       \\ \\\n\r");
					    printf("                \\.  |                       /  \\\n\r");
					    printf("                  \\ \\                      |._  |\n\r");
					    printf("                   \\ \\                     |  ()|\n\r");
					    printf("                    \\ \\                     \\  /\n\r");
					    printf("                     ) `-.                   | |\n\r");
					    printf("                    // .__)                  | |\n\r");
					    printf("                 _.//7'                      | |\n\r");
					    printf("               '---'                         j_| `\n\r");
					    printf("                                            (| |\n\r");
					    printf("                                             |  \\\n\r");
					    printf("                                             |lllj\n\r");
					    printf("                                             |||||\n\r");
					    printf("\r\nWchodzisz do pomieszczenia i widzisz zywy szkielet ktory zmierza w twoja strone \r\n");
					    walka(30, 6);
					    printf("\r\nZnajdujesz Miksture uzdrawiajaca ktora przywraca ci 35 punktow zdrowa\r\n");
					    zycie = zycie + 35;
					    wyswietlInterfejs();
				  }
				  printf("\r\nGdzie sie udajesz ?\r\n");
				  printf("4. lewo\r\n");
				  printf("6. prawo\r\n");
				  HAL_UART_Receive(&huart2, &gdzieidzie, 1, HAL_MAX_DELAY);
				  aktualny[wybor]=' ';
				  if (gdzieidzie == '4'){
						  if (klucze == 2){
					  	  	  wybor = 1;
				  	  	  }
					  	  else{
							printf("\r\nZauwazasz drzwi sa one wieksze w dodatku stworzone z metalu oraz maja dwie dziurki na klucze\r\n");
					  		printf("\n\r(Posiadasz za malo kluczy !!!)\r\n");
					  	  }
				  }
				  else if (gdzieidzie == '6'){
					  wybor = 3;
					  if (odwiedzone[3])
						  printf("\r\nOdwiedziles juz ten pokoj. Cialo rozplatanego pajaka lezy tak samo jak wczesniej\r\n");
				  }
				  else if (gdzieidzie == '2' || gdzieidzie == '8'){
					  sciana();
					  printf("\r\nJedyne co widzisz to sciana\r\n");
					  goto pokojdwa;
				  }
				  break;
			  case 3: // pajak
			  pokojtrzy:
			  aktualny[wybor]='X';
				  if (!odwiedzone[wybor])
				  {
					  odwiedzone[wybor] = 1;
					  printf("           ____                      ,\r\n");
					  printf("          /---.'.__             ____//\r\n");
					  printf("               '--.\\           /.---'\r\n");
					  printf("          _______  \\\\         //\r\n");
					  printf("        /.------.\\  \\|      .'/  ______\r\n");
					  printf("       //  ___  \\ \\ ||/|\\  //  _/_----.\\__\r\n");
					  printf("      |/  /.-.\\  \\ \\:|< >|// _/.'..\\   '--'\r\n");
					  printf("         //   \\'\\ | \\'|.'/ /_/ /  \\\\\r\n");
					  printf("        //     \\ \\_\\/' ' ~\\-'.-'    \\\\\r\n");
					  printf("       //       '-._| :H: |'-.__     \\\\\r\n");
					  printf("      //           (/'===\\)'-._\\     ||\r\n");
					  printf("      ||                        \\\\    \\|\r\n");
					  printf("      ||                         \\\\    '\r\n");
					  printf("      |/                          \\\\\r\n");
					  printf("                                   ||\r\n");
					  printf("                                   ||\r\n");
					  printf("                                   \\\\\r\n");
					  printf("                                    '\r\n");
					  printf("\r\nWchodzisz do pomieszczenia po czym skacze na ciebie ogromny pajak\r\n");
					  walka(25, 3);
					  printf("\r\nRozpalawiasz pajaka z ogromna sila\r\n");
				  }
				  printf("\r\nGdzie sie udajesz ?\r\n");
				  printf("4. lewo\r\n");
				  printf("8. gora\r\n");
				  printf("6. prawo\r\n");
				  HAL_UART_Receive(&huart2, &gdzieidzie, 1, HAL_MAX_DELAY);
				  aktualny[wybor]=' ';
				  if (gdzieidzie == '4'){
					  wybor = 2;
					  if (odwiedzone[2])
						  printf("\r\nJuz tutaj byles resztki szkieletu sa rozsypane po posadzce\r\n");
				  }
				  else if (gdzieidzie == '8'){
					  wybor = 4;
					  if (odwiedzone[4])
						  printf("\r\nOdwiedziles juz ten pokoj, podziurawiona imitacja skrzyni stoi dalej w tym samym miejscu \r\n");
				  }
				  else if (gdzieidzie == '6'){
					  printf("\r\nZnajdujesz sie w pokoju startowym\r\n");
					  wybor = 0;
				  }
				  else if (gdzieidzie == '2'){
					  sciana();
					  printf("\r\nJedyne co widzisz to sciana \n\r(PODPOWIEDZ : ludzie nie moge przechodzic przez sciany)\r\n");
					  goto pokojtrzy;
				  }
				  break;
			  case 4: // skrzynia mimic
			  pokojcztery:
			  aktualny[wybor]='X';
				  if (!odwiedzone[wybor])
				  {
					  printf("*******************************************************************************\n\r");
					  printf("          |                   |                  |                     |\n\r");
					  printf(" _________|________________.=\"\"_;=.______________|_____________________|_______\n\r");
					  printf("|                   |  ,-\"_,=\"\"     `\"=.|                  |\n\r");
					  printf("|___________________|__\"=._o`\"-._        `\"=.______________|___________________\n\r");
					  printf("          |                `\"=._o`\"=._      _`\"=._                     |\n\r");
					  printf(" _________|_____________________:=._o \"=._.\"_.-=\"'\"=.__________________|_______\n\r");
					  printf("|                   |    __.--\" , ; `\"=._o.\" ,-\"\"\"-._ \".   |\n\r");
					  printf("|___________________|_._\"  ,. .` ` `` ,  `\"-._\"-._   \". '__|___________________\n\r");
					  printf("          |           |o`\"=._` , \"` `; .\". ,  \"-._\"-._; ;              |\n\r");
					  printf(" _________|___________| ;`-.o`\"=._; .\" ` '`.\"\\` . \"-._ /_______________|_______\n\r");
					  printf("|                   | |o;    `\"-.o`\"=._``  '` \" ,__.--o;   |\n\r");
					  printf("|___________________|_| ;     (#) `-.o `\"=.`_.--\"_o.-; ;___|___________________\n\r");
					  printf("____/______/______/___|o;._    \"      `\".o|o_.--\"    ;o;____/______/______/____\n\r");
					  printf("/______/______/______/_\"=._o--._        ; | ;        ; ;/______/______/______/_\n\r");
					  printf("____/______/______/______/__\"=._o--._   ;o|o;     _._;o;____/______/______/____\n\r");
					  printf("/______/______/______/______/____\"=._o._; | ;_.--\"o.--\"_/______/______/______/_\n\r");
					  printf("____/______/______/______/______/____\"=.o|o_.--\"\"___/______/______/______/____\n\r");
					  printf("/______/______/______/______/______/______/______/______/______/______/______/_\n\r");
					  printf("*******************************************************************************\n\r");
					  printf("\r\nWchodzac do pomieszczenia zauwazasz skrzynie\r\n");
					  printf("Czy chcesz ja otworzyc ???\r\n");
					  printf("5. Otwieram !\r\n");
					  printf("0. Nie otwieram\r\n");
					  HAL_UART_Receive(&huart2, &otworzyc, 1, HAL_MAX_DELAY);
					  if (otworzyc == '5'){
						  printf("\n\rTO PULAPKA BRON SIE!\r\n");
						  walka(45, 6);
						  printf("\r\nPodstepna bestia udajaca skrzynie naraszcie przestaje sie ruszac\r\n");
						  odwiedzone[wybor] = 1;
					  }
				  }
				  printf("\r\nGdzie sie udajesz ?\r\n");
				  printf("2. dol\r\n");
				  HAL_UART_Receive(&huart2, &gdzieidzie, 1, HAL_MAX_DELAY);
				  aktualny[wybor]=' ';
				  if (gdzieidzie == '2'){
					  wybor = 3;
					  if (odwiedzone[3])
						  printf("\r\nOdwiedziles juz ten pokoj. Cialo rozplatanego pajaka lezy tak samo jak wczesniej\r\n");
				  }
				  else if (gdzieidzie == '8'){
					  printf("\r\n|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____||       _                         ||_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|___|       \`*-.                     |___|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____||        )  _`-.                  ||_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|___|       : _   '  \                |___|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____||       ; *` _.   `*-._           ||_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|___|       `-.-'          `-.        |___|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____||         ;       `       `.      ||_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|___|         :.       .        \     |___|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____||         . \  .   :   .-'   .    ||_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|___|         '  `+.;  ;  '      :    |___|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____||         :  '  |    ;       ;-.  ||_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|___|         ; '   : :`-:     _.`* ; |___|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____||      .*' /  .*' ; .*`- +'  `*'  ||_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|___|      `*-*   `*-*  `*-*'         |___|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____||_________________________________||_____|_____|_____|_____|_____|__\r\n");
					  	  printf("___|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|\r\n");
					  	  printf("|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|_____|__\r\n\r\n");
					    printf("\r\nPodchodzisz do sciany i widzisz slodki obrazek z kotkiem\n\r");
				  }
				  else if (gdzieidzie == '4' || gdzieidzie == '6'){
					  sciana();
					  printf("\r\nWidzisz sciane\r\n");
					  goto pokojcztery;
				  }
				  break;
			  case 5: // wojownik z kluczem
			  pokojpiec:
			  aktualny[wybor]='X';
				  if (!odwiedzone[wybor])
				  {
					  odwiedzone[wybor] = 1;
					  printf("                           __.--|~|--.__                               ,,;/;\r\n");
					  printf("                         /~     | |    ;~\\                          ,;;;/;;'\r\n");
					  printf("                        /|      | |    ;~\\\\                      ,;;;;/;;;'\r\n");
					  printf("                       |/|      \\_/   ;;;|\\                    ,;;;;/;;;;'\r\n");
					  printf("                       |/ \\          ;;;/  )                 ,;;;;/;;;;;'\r\n");
					  printf("                   ___ | ______     ;_____ |___....__      ,;;;;/;;;;;'\r\n");
					  printf("             ___.-~ \\(|\\  \\.\\ \\__/ /./ /:|)~   ~   \\   ,;;;;/;;;;;'\r\n");
					  printf("         /~~~    ~\\    |  ~-.     |   .-~: |//  _.-~~--,;;;;/;;;;;'\r\n");
					  printf("        (.-~___     \\.'|    | /-.__.-\\|::::| //~     ,;;;;/;;;;;'\r\n");
					  printf("        /      ~~--._ \\|   /          `\\:: |/      ,;;;;/;;;;;'\r\n");
					  printf("     .-|             ~~|   |  /V\"\"\"\"V\\ |:  |     ,;;;;/;;;;;'\r\n");
					  printf("    /                   \\  |  ~`^~~^'~ |  /    ,;;;;/;;;;;'\r\n");
					  printf("   (        \\             \\|`\\._____./'|/    ,;;;;/;;;;;' \\\r\n");
					  printf("  / \\        \\                             ,;;;;/;;;;;'     /\r\n");
					  printf(" |            |                          ,;;;;/;;;;;'      | \r\n");
					  printf("|`-._          |                       ,;;;;/;;;;;'              \\\r\n");
					  printf("|             /                      ,;;;;/;;;;;'  \\              \\__________\r\n");
					  printf("(             )                 |  ,;;;;/;;;;;'      |        _.--~\r\n");
					  printf(" \\          \\/ \\              ,  ;;;;;/;;;;;'       /(     .-~_..--~~~~~~~~~~\r\n");
					  printf(" \\__         '  `       ,     ,;;;;;/;;;;;'    .   /   :    \\/'/'       /|_/|   ``|\r\n");
					  printf(" /          \\'  |`._______ ,;;;;;;/;;;;;;' ~~~~'   .'    | |       /~ (/\\/    ||\r\n");
					  printf("| _.-~~~~-._ |   \\ __   .,;;;;;;/;;;;;;'          |    | |       / ~/_-'|-   /|\r\n");
					  printf("|/~ _.-~~~-._\\    /~/   ;;;;;;;/;;;;;;;'          |    | |      ,/)  /  /-   / |\r\n");
					  printf("(/~         \\| /'  |   ;;;;;;/;;;;;;;;            ;   | |      (.-~;  /-   /  |\r\n");
					  printf("|            /___ `-,;;;;;/;;;;;;;;'            |   | |     ,/)  /  /|- _/  //\r\n");
					  printf(" \\            \\  `-.`---/;;;;;;;;;' |          _'   | |   /'('  /  /|- _/  //\r\n");
					  printf("   \\           /~~/ `-. |;;;;;''    ______.--~~ ~\\  | |  ,~)')  /   | \\~-==//\r\n");
					  printf("     \\      /~(   `-\\  `-.`-;   /|    ))   __-####\\ | |   (,   /|    |  \\\r\n");
					  printf("       \\  /~.  `-.   `-.( `-.`~~ /##############'~~)| |   '   / |    |   ~\\\r\n");
					  printf("        \\(   \\    `-._ /~)_/|  /############'       | |      /  \\     \\_  `\r\n");
					  printf("        ,~`\\  `-._  / )#####|/############'   /     | |  _--~ _/ | .-~~____--'\r\n");
					  printf("       ,'\  `-._  ~)~~ `################'            | | ((~>/~   \\ ((((- -_\r\n");
					  printf("     ,'   `-.___)~~      `#############             | |           ~-_     ~\\\r\n");
					  printf(" _,.'        ,'           `###########              | |            _-~-__    (\r\n");
					  printf("|  `-.     ,'              `#########       \\       | |          ((.-~~~-~_--~\r\n");
					  printf("`\\    `-.;'                  `####\"                | |           \"     ((.-~~\r\n");
					  printf("  `-._   )               \\     |   |        .       |  \\                 \"\r\n");
					  printf("      `~~  _/                  |    \\               |   `---------------------\r\n");
					  printf("\r\nWchodzisz do pomieszczenia i widzisz nieumarlego wojownika z kluczem przewieszonym na szyi\r\n");
					  walka(50, 3);
					  printf("Udalo ci sie pociac nieumalego wojownika tak by juz wiecej ci nie zagrazal. Krwia brudzi on sciany\r\n");
					  printf("\n\rWyrywasz mu klucz z szyji i podazasz dalej\r\n");
					  klucze++;
				  }
				  printf("\r\nGdzie sie udajesz ?\r\n");
				  printf("8. gora\r\n");
				  printf("6. prawo\r\n");
				  HAL_UART_Receive(&huart2, &gdzieidzie, 1, HAL_MAX_DELAY);
				  aktualny[wybor]=' ';
				  if (gdzieidzie == '8'){
					  printf("\r\nZnajdujesz sie w pokoju startowym\r\n");
					  wybor = 0;
				  }
				  else if (gdzieidzie == '6'){
					  wybor = 7;
					  if (odwiedzone[7])
						  printf("\r\nByles juz tu, widzisz tylko przebitego pajaka\r\n");
				  }
				  else if (gdzieidzie == '2' || gdzieidzie == '4'){
					  sciana();
					  printf("\r\nZwykla sciana tylko strasznie ubrudzona krwia...\r\n");
					  goto pokojpiec;
				  }
				  break;
			  case 6: // szkielet
			  pokojszesc:
			  aktualny[wybor]='X';
				  if (!odwiedzone[wybor])
				  {
					    odwiedzone[wybor] = 1;
					    printf("                              _.--\"\"-._\n\r");
					    printf("  .                         .\"         \".\n\r");
					    printf(" / \\    ,^.         /(     Y             |      )\\\n\r");
					    printf("/   `---. |--'\\    (  \\__..'--   -   -- -'\"\"-.-'  )\n\r");
					    printf("|        :|    `>   '.     l_..-------.._l      .'\n\r");
					    printf("|      __l;__ .'      \"-.__.||_.-'v'-._||`\"----\"\n\r");
					    printf("\\  .-' | |  `              l._       _.'\n\r");
					    printf(" \\/    | |                   l`^^'^^'j\n\r");
					    printf("       | |                _   \\_____/     _\n\r");
					    printf("       | |               l `--__)-'(__.--' |\n\r");
					    printf("       | |               | /`---``-----'\"1 |  ,-----.\n\r");
					    printf("       | |               )/  `--' '---'   \\'-'  ___  `-. \n\r");
					    printf("     _ L |_            //  `-'  '`----'  /  /  |   |  `. \\\n\r");
					    printf("    '._' / \\         _/(   `/   )- ---' ;  /__.J   L.__.\\ :\n\r");
					    printf("      `._;/7(-.......'  /        ) (     |  |            | |\n\r");
					    printf("      `._;l _'--------_/        )-'/     :  |___.    _._./ ;\n\r");
					    printf("        | |                 .__ )-'\\  __  \\  \\  I   1   / /\n\r");
					    printf("        `-'                /   `-\\-(-'   \\ \\  `.|   | ,' /\n\r");
					    printf("                           \\__  `-'    __/  `-. `---'',-'\n\r");
					    printf("                              )-._.-- (        `-----'\n\r");
					    printf("                             )(  l\\ o ('..-.\n\r");
					    printf("                       _..--' _'-' '--'.-. |\n\r");
					    printf("                __,,-'' _,,-''            \\ \\\n\r");
					    printf("               f'. _,,-'                   \\ \\\n\r");
					    printf("              ()--  |                       \\ \\\n\r");
					    printf("                \\.  |                       /  \\\n\r");
					    printf("                  \\ \\                      |._  |\n\r");
					    printf("                   \\ \\                     |  ()|\n\r");
					    printf("                    \\ \\                     \\  /\n\r");
					    printf("                     ) `-.                   | |\n\r");
					    printf("                    // .__)                  | |\n\r");
					    printf("                 _.//7'                      | |\n\r");
					    printf("               '---'                         j_| `\n\r");
					    printf("                                            (| |\n\r");
					    printf("                                             |  \\\n\r");
					    printf("                                             |lllj\n\r");
					    printf("                                             |||||\n\r");
					    printf("\r\nWchodzisz do pomieszczenia i widzisz zywy szkielet ktory zmierza w twoja strone \r\n");
					    walka(25, 5);
					    printf("\r\nLamiesz przeciwnikowi kosci, bo co innego mozna polamac szkieletowi\r\n");
				  }
				  printf("\r\nGdzie sie udajesz ?\r\n");
				  printf("4. lewo\r\n");
				  printf("8. gora\r\n");
				  printf("6. prawo\r\n");
				  printf("2. dol\r\n");
				  HAL_UART_Receive(&huart2, &gdzieidzie, 1, HAL_MAX_DELAY);
				  aktualny[wybor]=' ';
				  if (gdzieidzie == '4'){
					  printf("\r\nZnajdujesz sie w pokoju startowym\r\n");
					  wybor = 0;
				  }
				  else if (gdzieidzie == '8'){
					  wybor = 9;
					  if (odwiedzone[9])
						  printf("\r\nOdwiedziles juz ten pokoj, glowa wojownika jest oddzielona od reszty tulowia\r\n");
				  }
				  else if (gdzieidzie == '6'){
					  wybor = 11;
					  if (odwiedzone[11])
						  printf("\r\nJuz sie napiles, wiecej ci sie nie chce\n\r");
				  }
				  else if (gdzieidzie == '2'){
				  	  wybor = 7;
					  if (odwiedzone[7])
						  printf("\r\nByles juz tu, widzisz tylko przebitego pajaka\r\n");
				  }
				  else{
					  goto pokojszesc;
				  }
				  break;
			  case 7: // pajak
			  pokojsiedem:
			  aktualny[wybor]='X';
				  if (!odwiedzone[wybor])
				  {
					  odwiedzone[wybor] = 1;
					  printf("           ____                      ,\r\n");
					  printf("          /---.'.__             ____//\r\n");
					  printf("               '--.\\           /.---'\r\n");
					  printf("          _______  \\\\         //\r\n");
					  printf("        /.------.\\  \\|      .'/  ______\r\n");
					  printf("       //  ___  \\ \\ ||/|\\  //  _/_----.\\__\r\n");
					  printf("      |/  /.-.\\  \\ \\:|< >|// _/.'..\\   '--'\r\n");
					  printf("         //   \\'\\ | \\'|.'/ /_/ /  \\\\\r\n");
					  printf("        //     \\ \\_\\/' ' ~\\-'.-'    \\\\\r\n");
					  printf("       //       '-._| :H: |'-.__     \\\\\r\n");
					  printf("      //           (/'===\\)'-._\\     ||\r\n");
					  printf("      ||                        \\\\    \\|\r\n");
					  printf("      ||                         \\\\    '\r\n");
					  printf("      |/                          \\\\\r\n");
					  printf("                                   ||\r\n");
					  printf("                                   ||\r\n");
					  printf("                                   \\\\\r\n");
					  printf("                                    '\r\n");
					  printf("\r\nWchodzisz do pokoju i odrazu rzuca sie na ciebie ogromny pajak\r\n");
					  walka(30, 4);
					  printf("\r\nZadajesz pajakowi smiertelny cios po ktorym napewno sie nie podniesie\n\r");
				  }
				  printf("\r\nGdzie sie udajesz ?\r\n");
				  printf("4. lewo\r\n");
				  printf("8. gora\r\n");
				  printf("2. dol\r\n");
				  HAL_UART_Receive(&huart2, &gdzieidzie, 1, HAL_MAX_DELAY);
				  aktualny[wybor]=' ';
				  if (gdzieidzie == '4'){
					  wybor = 5;
					  if (odwiedzone[5])
						  printf("\r\nOdwiedziles juz ten pokoj, resztki nieumarlego wojownika rozrzucone sa po pomieszczeniu\r\n");
				  }
				  else if (gdzieidzie == '8'){
					  wybor = 6;
					  if (odwiedzone[6])
						  printf("\r\nTutaj pokonales szkieleta na co wskazuje polamany szkielet lezacy na podlodze\r\n");
				  }
				  else if (gdzieidzie == '2'){
					  wybor = 8;
					  if (odwiedzone[8])
						  printf("\r\nOdwiedziles juz ten pokoj, jedyne co widzisz na ten moment to pusta skrzynia, aczkolwiek kto wie co tu sie kryje\r\n");
				  }
				  else if (gdzieidzie == '6'){
					  sciana();
					  printf("\r\nWidzisz sciane\r\n");
					  goto pokojsiedem;
				  }
				  break;
			  case 8: // pokoj ze skrzynia do otwarcia w ktorej jest mieczyk +3 obrazen
			  pokojosiem:
			  aktualny[wybor]='X';
				  if (!odwiedzone[wybor])
				  {
					  printf("*******************************************************************************\n\r");
					  printf("          |                   |                  |                     |\n\r");
					  printf(" _________|________________.=\"\"_;=.______________|_____________________|_______\n\r");
					  printf("|                   |  ,-\"_,=\"\"     `\"=.|                  |\n\r");
					  printf("|___________________|__\"=._o`\"-._        `\"=.______________|___________________\n\r");
					  printf("          |                `\"=._o`\"=._      _`\"=._                     |\n\r");
					  printf(" _________|_____________________:=._o \"=._.\"_.-=\"'\"=.__________________|_______\n\r");
					  printf("|                   |    __.--\" , ; `\"=._o.\" ,-\"\"\"-._ \".   |\n\r");
					  printf("|___________________|_._\"  ,. .` ` `` ,  `\"-._\"-._   \". '__|___________________\n\r");
					  printf("          |           |o`\"=._` , \"` `; .\". ,  \"-._\"-._; ;              |\n\r");
					  printf(" _________|___________| ;`-.o`\"=._; .\" ` '`.\"\\` . \"-._ /_______________|_______\n\r");
					  printf("|                   | |o;    `\"-.o`\"=._``  '` \" ,__.--o;   |\n\r");
					  printf("|___________________|_| ;     (#) `-.o `\"=.`_.--\"_o.-; ;___|___________________\n\r");
					  printf("____/______/______/___|o;._    \"      `\".o|o_.--\"    ;o;____/______/______/____\n\r");
					  printf("/______/______/______/_\"=._o--._        ; | ;        ; ;/______/______/______/_\n\r");
					  printf("____/______/______/______/__\"=._o--._   ;o|o;     _._;o;____/______/______/____\n\r");
					  printf("/______/______/______/______/____\"=._o._; | ;_.--\"o.--\"_/______/______/______/_\n\r");
					  printf("____/______/______/______/______/____\"=.o|o_.--\"\"___/______/______/______/____\n\r");
					  printf("/______/______/______/______/______/______/______/______/______/______/______/_\n\r");
					  printf("*******************************************************************************\n\r");
					  printf("\r\nWchodzac do pomieszczenia zauwazasz skrzynie\r\n");
					  printf("Czy chcesz ja otworzyc ???\r\n");
					  printf("5. Otwieram !\r\n");
					  printf("0. Nie otwieram\r\n");
					  HAL_UART_Receive(&huart2, &otworzyc, 1, HAL_MAX_DELAY);
					  if (otworzyc == '5'){
						  odwiedzone[wybor] = 1;
						  printf("%50s", "Gratulacje zdobywasz Legandarny Miecz - Durendal !!!\r\n");
				            printf("             _   \n\r");
				            printf("  _         | |   \n\r");
				            printf(" | | _______| |---------------------------------------------\\   \n\r");
				            printf(" |:-)_______|==[]============================================>   \n\r");
				            printf(" |_|        | |---------------------------------------------/   \n\r");
				            printf("            |_|   \n\r");
						  dmgGracza += 3;
					  }
				  }
				  printf("\r\nGdzie sie udajesz ?\r\n");
				  printf("8. gora\r\n");
				  HAL_UART_Receive(&huart2, &gdzieidzie, 1, HAL_MAX_DELAY);
				  aktualny[wybor]=' ';
				  if (gdzieidzie == '8'){
					  wybor = 7;
					  if (odwiedzone[7])
						  printf("\r\nByles juz tu, widzisz tylko przebitego pajaka\r\n");
				  }
				  else if (gdzieidzie == '2' || gdzieidzie == '4' || gdzieidzie == '6'){
					  sciana();
					  printf("\r\nPrzegladasz ten kat ale no niestety nic tu nie ma\r\n");
					  goto pokojosiem;
				  }
				  break;
			  case 9: // wojownik potion leczacy
			  pokojdziewiec:
			  aktualny[wybor]='X';
				  if (!odwiedzone[wybor])
				  {
					  odwiedzone[wybor] = 1;
					  printf("                           __.--|~|--.__                               ,,;/;\r\n");
					  printf("                         /~     | |    ;~\\                          ,;;;/;;'\r\n");
					  printf("                        /|      | |    ;~\\\\                      ,;;;;/;;;'\r\n");
					  printf("                       |/|      \\_/   ;;;|\\                    ,;;;;/;;;;'\r\n");
					  printf("                       |/ \\          ;;;/  )                 ,;;;;/;;;;;'\r\n");
					  printf("                   ___ | ______     ;_____ |___....__      ,;;;;/;;;;;'\r\n");
					  printf("             ___.-~ \\(|\\  \\.\\ \\__/ /./ /:|)~   ~   \\   ,;;;;/;;;;;'\r\n");
					  printf("         /~~~    ~\\    |  ~-.     |   .-~: |//  _.-~~--,;;;;/;;;;;'\r\n");
					  printf("        (.-~___     \\.'|    | /-.__.-\\|::::| //~     ,;;;;/;;;;;'\r\n");
					  printf("        /      ~~--._ \\|   /          `\\:: |/      ,;;;;/;;;;;'\r\n");
					  printf("     .-|             ~~|   |  /V\"\"\"\"V\\ |:  |     ,;;;;/;;;;;'\r\n");
					  printf("    /                   \\  |  ~`^~~^'~ |  /    ,;;;;/;;;;;'\r\n");
					  printf("   (        \\             \\|`\\._____./'|/    ,;;;;/;;;;;' \\\r\n");
					  printf("  / \\        \\                             ,;;;;/;;;;;'     /\r\n");
					  printf(" |            |                          ,;;;;/;;;;;'      | \r\n");
					  printf("|`-._          |                       ,;;;;/;;;;;'              \\\r\n");
					  printf("|             /                      ,;;;;/;;;;;'  \\              \\__________\r\n");
					  printf("(             )                 |  ,;;;;/;;;;;'      |        _.--~\r\n");
					  printf(" \\          \\/ \\              ,  ;;;;;/;;;;;'       /(     .-~_..--~~~~~~~~~~\r\n");
					  printf(" \\__         '  `       ,     ,;;;;;/;;;;;'    .   /   :    \\/'/'       /|_/|   ``|\r\n");
					  printf(" /          \\'  |`._______ ,;;;;;;/;;;;;;' ~~~~'   .'    | |       /~ (/\\/    ||\r\n");
					  printf("| _.-~~~~-._ |   \\ __   .,;;;;;;/;;;;;;'          |    | |       / ~/_-'|-   /|\r\n");
					  printf("|/~ _.-~~~-._\\    /~/   ;;;;;;;/;;;;;;;'          |    | |      ,/)  /  /-   / |\r\n");
					  printf("(/~         \\| /'  |   ;;;;;;/;;;;;;;;            ;   | |      (.-~;  /-   /  |\r\n");
					  printf("|            /___ `-,;;;;;/;;;;;;;;'            |   | |     ,/)  /  /|- _/  //\r\n");
					  printf(" \\            \\  `-.`---/;;;;;;;;;' |          _'   | |   /'('  /  /|- _/  //\r\n");
					  printf("   \\           /~~/ `-. |;;;;;''    ______.--~~ ~\\  | |  ,~)')  /   | \\~-==//\r\n");
					  printf("     \\      /~(   `-\\  `-.`-;   /|    ))   __-####\\ | |   (,   /|    |  \\\r\n");
					  printf("       \\  /~.  `-.   `-.( `-.`~~ /##############'~~)| |   '   / |    |   ~\\\r\n");
					  printf("        \\(   \\    `-._ /~)_/|  /############'       | |      /  \\     \\_  `\r\n");
					  printf("        ,~`\\  `-._  / )#####|/############'   /     | |  _--~ _/ | .-~~____--'\r\n");
					  printf("       ,'\  `-._  ~)~~ `################'            | | ((~>/~   \\ ((((- -_\r\n");
					  printf("     ,'   `-.___)~~      `#############             | |           ~-_     ~\\\r\n");
					  printf(" _,.'        ,'           `###########              | |            _-~-__    (\r\n");
					  printf("|  `-.     ,'              `#########       \\       | |          ((.-~~~-~_--~\r\n");
					  printf("`\\    `-.;'                  `####\"                | |           \"     ((.-~~\r\n");
					  printf("  `-._   )               \\     |   |        .       |  \\                 \"\r\n");
					  printf("      `~~  _/                  |    \\               |   `---------------------\r\n");
					  printf("\r\nWchodzisz do pomieszczenia gdzie odrazu w twoja strone rusza nieumarly wojownik !\r\n");
					  walka(50, 3);
					  printf("\r\nOdcinasz nieumarlemu przeciwnikowi glowe, a ten upada na kolana\r\n");
					  printf("Po walce udaje ci sie znalezc miksture leczenia. Przywracasz sobie 35 punktow zdrowia !\r\n");
					  zycie = zycie + 35;
					  wyswietlInterfejs();
					  printf("\r\nZauwazasz ze drzwi u gory sa wieksze oraz stworzone z metalu\r\n");
				  }
				  printf("\r\nGdzie sie udajesz ?\r\n");
				  printf("8. gora\r\n");
				  printf("2. dol\r\n");
				  HAL_UART_Receive(&huart2, &gdzieidzie, 1, HAL_MAX_DELAY);
				  aktualny[wybor]=' ';
				  if (gdzieidzie == '8'){
					  wybor = 10;
					  if (odwiedzone[10])
						  printf("\r\nWidzisz nieruchomego mechanicznego Centuriona. Wyglada na popsutego, ciekawe przez kogo ???\n\r");
				  }
				  else if (gdzieidzie == '2'){
					  wybor = 6;
					  if (odwiedzone[6])
						  printf("\r\nTutaj pokonales szkieleta na co wskazuje polamany szkielet lezacy na podlodze\r\n");
				  }
				  else if (gdzieidzie == '6' || gdzieidzie == '4'){
					  sciana();
					  printf("\r\nAle sliczne te sciany takie rowniutkie\r\n");
					  goto pokojdziewiec;
				  }
				  break;
			  case 10: // centurion wielki bydlak kuuuuurde z kluczem
			  pokojdziesiec:
			  aktualny[wybor]='X';
				  if (!odwiedzone[wybor])
				  {
					  odwiedzone[wybor] = 1;
					  printf("          {}\r\n");
					  printf("         .--.\r\n");
					  printf("        /.--.\\\r\n");
					  printf("        |====|\r\n");
					  printf("        |`::`|\r\n");
					  printf("    .-;`\\..../`;-.\r\n");
					  printf("   /  |...::...|  \\\r\n");
					  printf("  |   /'''::'''\\   |\r\n");
					  printf("  ;--'\\   ::   /\\--;\r\n");
					  printf("  <__>,>._::_.<,<__>\r\n");
					  printf("  |  |/   ^^   \\|  |\r\n");
					  printf("  \\::/|        |\\::/\r\n");
					  printf("  |||\\|        |/|||\r\n");
					  printf("  ''' |___/\\___| '''\r\n");
					  printf("       \\_ || _/\r\n");
					  printf("       <_ >< _>\r\n");
					  printf("       |  ||  |\r\n");
					  printf("       |  ||  |\r\n");
					  printf("      _\\.:||:./_\r\n");
					  printf("     /____/\\____\\\r\n");
					  printf("\r\nOtwierasz wielkie metalowe drzwi za ktorymi dostrzegasz ogromna stalowa machine.\r\n");
					  printf("To krasnoludzki centurion, a w dodatku zauwazasz ze pilnuje on klucza\r\n");
					  walka(60, 6);
					  printf("\r\nPowalasz wielkiego stalowego Centuriona na ziemie, po czym odbierasz mu klucz\n\r");
					  printf("(Zdobywasz klucz)\n\r");
					  klucze++;
				  }
				  printf("\r\nGdzie sie udajesz ?\r\n");
				  printf("2. dol\r\n");
				  HAL_UART_Receive(&huart2, &gdzieidzie, 1, HAL_MAX_DELAY);
				  aktualny[wybor]=' ';
				  if (gdzieidzie == '2'){
					  wybor = 9;
					  if (odwiedzone[9])
						  printf("\r\nOdwiedziles juz ten pokoj, glowa wojownika jest oddzielona od reszty tulowia\r\n");
				  }
				  else if (gdzieidzie == '4' || gdzieidzie == '8' || gdzieidzie == '6'){

					      printf("\n\r/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\\r\n");
					      printf("\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\\r\n");
					      printf(" \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/\r\n");
					      printf("/\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\r\n");
					      printf("/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\\r\n");
					      printf("\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\\r\n");
					      printf(" \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/\r\n");
					      printf("/\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\r\n");
					      printf("/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\\r\n");
					      printf("\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\\r\n");
					      printf(" \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/\r\n");
					      printf("/\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\r\n");
					      printf("/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\\r\n");
					      printf("\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\\r\n");
					      printf(" \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/\r\n");
					      printf("/\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\r\n");
					      printf("/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\\r\n");
					      printf("\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\\r\n");
					      printf(" \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/\r\n");
					      printf("/\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\r\n");
					      printf("/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\/__\\\r\n");
					      printf("\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\  /\\\r\n");
					      printf(" \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/  \\/\r\n\r\n");
					  printf("\r\nSciany w pokoju z Centurionem sa wyjatkowo zadbane oraz sa z innego materialu\r\n");
					  goto pokojdziesiec;
				  }
				  break;
			  case 11: // zrodelko manyyyyy odnawia po 1 z kazdych spelli
			  pokojjedenascie:
			  aktualny[wybor]='X';
				  if (!odwiedzone[wybor])
				  {
					  odwiedzone[wybor] = 1;
					  printf("\r\nWchodzisz do przyciemnionego pomieszczenia i dostrzegasz male zrodlo bijace ze skaly.\r\n");
					  printf("Chcesz sie z niego napic ?\r\n");
					  printf("5. No pewnie ze pije !\r\n");
					  printf("0. Moze lepiej nie.\r\n");
					  HAL_UART_Receive(&huart2, &otworzyc, 1, HAL_MAX_DELAY);
					  if (otworzyc == '5'){
						  printf("\r\nCzujesz ze magiczne zrodlo wzmocnilo twoje cialo \r\n(otrzymujesz po ladunku kazdej umiejetnosci)\r\n");
						  skillMieczUzycia += 1;
						  skillTarczaUzycia += 1;
						  skillCzarUzycia += 1;
					  	  wyswietlInterfejs();
					  }
				  }
				  else
				  {
					  printf("\r\nOdwiedziles juz ten pokoj\r\n");
				  }
				  printf("\r\nGdzie sie udajesz ?\r\n");
				  printf("4. lewo\r\n");
				  HAL_UART_Receive(&huart2, &gdzieidzie, 1, HAL_MAX_DELAY);
				  aktualny[wybor]=' ';
				  aktualny[wybor]=' ';
				  if (gdzieidzie == '4'){
					  wybor = 6;
					  if (odwiedzone[6])
						  printf("\r\nTutaj pokonales szkieleta na co wskazuje polamany szkielet lezacy na podlodze\r\n");
				  }
				  else if (gdzieidzie == '2' || gdzieidzie == '8' || gdzieidzie == '6'){
					  sciana();
					  printf("\r\nW tym pokoju sciany sa takie same jak wszedzie tylko bardziej mokre co najwyzej\r\n");
					  goto pokojjedenascie;
				  }
				  break;
			  default:
				  printf("\r\nTaki wybor nie istnieje\r\n");
				  break;
			  }
	      }
	  if (odwiedzone[1]){
		    printf("\n\r\n\r\n\r  ________          ____     _______ _____  _____ _________          ______  \n\r");
		    printf("|___  /\\ \\        / /\\ \\   / / ____|_   _|/ ____|__   __\\ \\        / / __ \\ \n\r");
		    printf("    / /  \\ \\  /\\  / /  \\ \\_/ / |      | | | (___    | |   \\ \\  /\\  / / |  | |\n\r");
		    printf("   / /    \\ \\/  \\/ /    \\   /| |      | |  \\___ \\   | |    \\ \\/  \\/ /| |  | |\n\r");
		    printf("  / /__    \\  /\\  /      | | | |____ _| |_ ____) |  | |     \\  /\\  / | |__| |\n\r");
		    printf(" /_____|    \\/  \\/       |_|  \\_____|_____|_____/   |_|      \\/  \\/   \\____/ \n\r");
		    printf("\n\r          GRATULACJE UDAlO CI SIE POKONAC NAJWYZSZEGO STRAZNIKA\n\r\n\r\n\r\n\r");

	  }
	  else{
		    printf("\n\r\n\r\n\r  _____   ____  _____             _______  __          \n\r");
		    printf(" |  __ \\ / __ \\|  __ \\     /\\    |___  / |/ /    /\\    \n\r");
		    printf(" | |__) | |  | | |__) |   /  \\      / /| ' /    /  \\   \n\r");
		    printf(" |  ___/| |  | |  _  /   / /\\ \\    / / |  <    / /\\ \\  \n\r");
		    printf(" | |    | |__| | | \\ \\  / ____ \\  / /__| . \\  / ____ \\ \n\r");
		    printf(" |_|     \\____/|_|  \\_\\/_/    \\_\\/_____|_|\\_\\/_/    \\_\\\n\r");
		    printf("\R\NCiemnosc cie pochlonela Paladynie :C\r\n\n\r\n\r\n\r");
	  }
	  char bufferKoniec[16]="KONIEC GRY";
	      lcd_clear();
	      while(1)
	      {
	          lcd_print(1,1,bufferKoniec);
	          for(int i=0;i<6;i++)
	          {
	          lcd_cmd(_SHIFT_RIGHT);
	          HAL_Delay(300);
	          }
	      }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc1, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LD2_Pin|LCD_D7_Pin|LCD_RS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_D6_Pin|LCD_D5_Pin|LCD_D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LCD_EN_GPIO_Port, LCD_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BT1_Pin */
  GPIO_InitStruct.Pin = BT1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BT1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD2_Pin LCD_D7_Pin LCD_RS_Pin */
  GPIO_InitStruct.Pin = LD2_Pin|LCD_D7_Pin|LCD_RS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_D6_Pin LCD_D5_Pin LCD_D4_Pin */
  GPIO_InitStruct.Pin = LCD_D6_Pin|LCD_D5_Pin|LCD_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : LCD_EN_Pin */
  GPIO_InitStruct.Pin = LCD_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_EN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

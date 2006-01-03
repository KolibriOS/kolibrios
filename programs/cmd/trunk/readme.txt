  CMD - Интерпретатор командной строки для Menuet
        Написано by Chemist dmitry_gt@tut.by

        Версия 0.27

	Русский текст немного пониже.

-----------------------------------------------------------------------

  CMD - Command Line Interpreter for Menuet
	Copyleft Chemist - dmitry_gt@tut.by

	Version 0.27

	As my english is not well, I will tell a few words about this
	program. It supports 14 commands, such as cp (copy file),
	rn (rename file), ps (process list), kill (kill process) etc.
	You can type 'help' after starting this shell to examine with
	other commands. Also it uses several special symbols, such as
	&, /, . and +. Use & sumbol to enter params. For example,
	type tinypad&cmd.asm to open cmd.asm file in tinypad editor.
	/ symbol is used to run special command scripts for CMD.
	It's syntax is: /filename without extension (For example,
	type /autoexec to execute autoexec.cmd file for CMD). And +
	symbol is used in cp and rn commands. You must type
	cp example.asm+example.bak to copy example.asm file to
	example.bak. Use dot to launch program, if CMD command and
	other external command match. For example, type help for
	'help' command or type .help to run 'help' program.

	This shell supports executeing special command
	scripts (something like .BAT files in MS-DOS). This files
	have an .CMD extinsion and u must use / symbol to execute it.
	U can use any CMD commands and filenames in this scripts.
	But you can't run other scripts from any CMD script (/ symbol).
	I recommed to use tipypad editor to edit this scripts and do
	not leave spaces and other symbols after commands in the script
	file, because it's executeing is not very developed yet.
	And it's strongly recommended to use ends command in the end
	of the script, because there are some problems with redrawing
	the window after executing scripts without this command.
	Every time when you launch CMD autoexec.cmd file automatically
	executes. You can edit or delete this file if you want.

	This version of CMD shell supports IPC. It mean, than you can
	write your own programs for this shell. Look at .ASM files in
	the Examples directory in this archive. I think that you will
	understand them without any difficulties. Do not forget, that
	you need CMDIPC.ASM file to compile this sources. It's
	recommended to compile it with MACROS.INC file (included in
	this archieve) to make their size smaller.

	And now about some bugs etc.

	----------------------------

	I've noticed, that there are some difficulties with files with
	1 or 2 symbols in extension. I recommend do not use such files
	not only in CMD, but almost in every programs in MeOS. It's
	possible to create such file in tinypad, and then (in MS-DOS or
	Windows) Scandisk will find errors in filenames etc. CMD do
	not support properly such filenames now.

	In CMDIPC.INC and CMD.ASM I used 5-th function (pause), because
	I need it when CMD communicates with IPC programs. It's
	even possible, that it will not enough 1/100 sec. on slow PC's,
	because CMD need this time to finish IPC-command from other
	IPC-program. U can change ebx value in CMDIPC.ASM in pause1:
	if you want. But it slows communication betwen CMD and
	IPC-programs for CMD. I hope, that you understand my words. :)

	Now (in this version) you can launch only one CMD shell.
	Because it's difficult to make communication betwen several
	copyes of CMD in memory and IPC-programs. I will solve this
	problem in future.

	I've tested this shell only on Ivan Poddubny's russian
	distributive. Also I recommend you to use it. :) CMD shell
	is included into this distributive (but possible not it's
	final version).

	Source code of this program is not optimized yet. I have such
	manner of programming from my childhood that my source code
	is not very "compact"

	etc. ;-)

	And now other information:
	--------------------------

	I do not want CMD design to change. I like white symbols on
	the black background. If you want to change the source and
	redistribute it, please, do not change it's design. :)

	If you will find some bugs or you have some wishes (or even you
	correct some mistakes in my english texts) -
	email me: dmitry_gt@tut.by

	And this program is distributed "as is". Use it on your own
	risk. ;-)

	And again - my English is not well. :)

	That's all!

	19.06.2004,
	Chemist
	
	Now russian text :)

-----------------------------------------------------------------------
 
  Доступные команды:

	ls [filename] - вывод списка файлов. Если после команды 
                        указать имя файла, то команда проверит его 
                        на наличие.

	cp [source_file+destination_file] - команда для 
                        копирования файлов. Проверяет файлы на 
                        наличие, и в случае ошибки выводит 
                        соответствующие сообщения.

        rn [source_file+destination_file] - команда для 
                        переименования файлов. Так же 
                        проверяет файлы на наличие, и в 
                        случае ошибки выводит соответствующие
			сообщения.

	ps            - вывод информации о процессах в системе.

        kill          - прервать процесс в системе. Внимание, 
                        после команды следует вводить 4-х 
                        значный номер процесса, а не PID. Номер 
                        процесса указан в последней колонке при 
                        вызове команды ps.

	help          - вывод краткой справочной информации.

        ver           - вывод используемой версии интерпретатора.

        cls           - очистка экрана.

        exit          - выход из интерпретатора.

        del [filename] - удалить файл с рамдиска. При отсутствии 
                        запрашиваемого файла будет выведено 
                        соответствующие сообщение об ошибке.

	shutdown      - завершить работу системы.

	pause         - ожидать нажатие клавиши. Используется для 
                        "взаимодействия" пользователя с командными 
                        скриптами для консоли (интерпретатора). 
                        Например, можно использовать данный скрипт 
                        
                        echo Do you want to delete cmd.asm?
                        pause
                        del cmd.asm

                        для того, чтобы поинтересоваться мнением 
                        пользователя, хочет ли он удалить файл cmd.asm 
                        или прервать работу скрипта.

	pause >nul	То же самое, только без вывода строки
			'Press ane key to continue (ESC - cancel)'

	echo [text]     - вывод текста на экран. Предназначена для 
                        подачи пользователю информации из командного 
                        скрипта. Если ввести команду echo без текста, 
                        то это просто вызовет переход на следующую 
                        строку.

	ends            - команда, доступная только из исполняемых 
                        скриптов. Служит для их корректного завершения,
                        т.к. иногда командные скрипты, у которых в 
                        конце стояли ненужные пробелы или символы EOL, 
                        вызывали проблемы с перерисовкой окна. Скорее 
                        всего эта команда присутствует в консоли 
                        временно и будет убрана после решения этой 
                        проблемы.

-----------------------------------------------------------------------
	
  Управляющие символы:

	/[командный скрипт] - предназначен для вызова на исполнение 
                        командного скрипта из консоли. Не может 
                        использоваться в непосредственно в самих 
                        командных скриптах. Если расширение испол-
                        няемого скрипта .cmd, то его указывать 
                        необязательно.

	& - данный символ используется для передачи вызываемой 
            программе параметров. Например, команда tinypad&cmd.asm 
            передаст программе tinypad параметр cmd.asm, 
            соответственно, tinypad откроет файл cmd.asm.

	+ - разделяет имена исходного и результирующего файлов в 
            командах cp и rn.

	. - запускает указанную после точки программу, даже если она
	    совпадает с командой CMD. Например, help - выполнить
	    команду 'help', но .help - запустить внешнюю программу
	    'help' (хотя такой пока вроде и нету :).

-----------------------------------------------------------------------

  Навигация по консоли:

	Для редактирования командной строки используются клавиши ESC, 
        BACKSPACE. ESC - для удаления всей командной строки, 
        BACKSPACE - для удаления последнего введенного символа. По 
        моему мнению, использование таких клавиш, как HOME, END, ARROW 
        KEY etc. не имеет смысла, т.к. вводимые команды слишком 
        просты и не требуют подробного редактирования. Поэтому я 
        оставил все примерно так, как было в MS-DOS 6.22.

        Клавиша UPARROW используется для повтора последней введенной 
        команды.

-----------------------------------------------------------------------

  Использование IPC во внешних программах:

	Вы можете писать программы, взаимодействующие с CMD через IPC.
	В основном, я полагаю, это может пригодится для написания
	консольных приложений для самого CMD (хотя возможно и другое).
	
	Для этого, к вашей программе вам необхрдимо подключить файл
	CMDIPC.INC (Естественно, от этого размер программы после
	компиляции несколько увеличится). После этого вам станут
	доступны 6 функции IPC, которые вы можете вызывать с помощью
	call из своей же программы. Вот их описание:

	---------------------------------------------------------------

	initipc - инициализация IPC для работы с CMD.

	call initipc - инициализировать IPC для работы с CMD.

	ВНИМАНИЕ! Используйте в самом начале программы. CMD будет ждать
		  только 10/100 секунды для того, чтобы получить
		  сообщение по IPC (хотя этого вполне достаточно).

	---------------------------------------------------------------

	print   - вывести строку в экран CMD.

	mov eax,strlen
	mov ebx,string
	call print

	Где strlen - длина строки в байтах,
	string     - указатель на строку.

	call print - вывод строки.

	---------------------------------------------------------------

	cls     - очистить экран CMD.

	call cls - вызвать очистку экрана.

	---------------------------------------------------------------

	eol     - пропустить строку.

	call eol - вызвать пропус строки.

	---------------------------------------------------------------

	getkey  - считать код нажатой клавиши в CMD.

	call getkey - ожидать нажатия клавиши и считать ее код.

	Вывод: byte [key] - код нажатой клавиши.

	ВНИМАНИЕ: После запуска программы из CMD, которая
		  поддерживает IPC, то окно CMD активируется сразу
		  после запуска программы. Поэтому, уже не нужно
		  тыкать мышкой на окно для того, чтобы ввести
		  что-нибудь в консоль, как это было в прошлых версиях.

	---------------------------------------------------------------

	endipc  - Завершить работу IPC-программы.

	call endipc - завершить программу.

	---------------------------------------------------------------

	Вообщем, вы можете посмотреть файл HELLO.ASM, который лежит в
	этом архиве. Там все должно быть понятно. Естественно, никто
	вам не мешает использовать параметры в IPC-программах для CMD.
	Смотрите пример PARAM.ASM.

	Кстати, в самом CMD и в CMDIPC.INC используется 5-я функция
	(пауза) для того, чтобы	дать время друг-другу выполнить
	требуемые от них через IPC действия. И если у вас комп сильно
	тормознючий, то возможно, выделенного времени будет
	недостаточно. Это не сложно полечить, увеличив значение
	ebx перед вызовом функции ядра (eax,5 - пауза). В противном
	случае могут повылетать вызовы IPC или что-нибудь вообще
	зависнет (первое - вероятнее). Но, естественно, ничего
	страшного в этом нет. ;-)

-----------------------------------------------------------------------

  Известные ошибки и недоработки:
	
	При работе с консолью CMD и системой MenuetOS вообще, я 
	заметил,что некоторые созданные в Menuet файлы не 
	воспринимаются MS-DOS. Это в первую очередь касается файлов, 
	с именами типа 1.1, b.bb и т.д. Поэтому возможны различные 
	тупиковые ситуации при взаимодействии созданных или 
	копированных файлов в MeOS с восприятием их в MS-DOS и Windows
	системах. И касается это не только CMD, но, например, и 
	тинипада. Поэтому я рекомендую использовать в Menuet файлы или
	без расширения вообще, или с полным расширением (занимающим
	все 3 байта, т.е. filename.ext,	а не filename.ex). В принципе
	механизм команд LS, LS имя_файла, CP, DEL и RN изменен, но
	пока нет совместимости между различными	программами, 
	работающими с файлами в Menuet. Т.е. даже возможно,
	что вы не сможете работать в консоли с файлом, созданном в 
	тинипаде, а потом skandisk вообще выдаст ошибку при проверке 
	файловой структуры дискеты, когда наткнется на этот файл, и он 
	не будет читаем из windows или MS-DOS. В принципе, я тестировал 
	программу только на моем PC, поэтому возможно всё. :)

	P.S. Команда LS покажет полностью все файлы на рамдиске, в том
	числе те, которые созданы неправильно и не будут работать в
	MS-DOS, LS имя_файла и т.д. может уже их не заметить. В 
	принципе такая же картина будет и в windows. Т.е. вы сможете
	лицезреть имена этих файлов, например, в windows commander'e,
	но блокнот их не откроет.
       
	Для того, чтоба дать время CMD обработать IPC-запрос, в
	CMDIPC.ASM мне пришлось использовать 5-ю функцию (пауза),
	поэтому немного замедляется время получения новых IPC-запросов.

	В данной версии вы не можете запускать больше одного
	терминала CMD одновременно. Это связано с тем, что я еще не
	добавил возможности работы нескольких консолей с
	IPC-программами одновременно.

	Код программы на данный момент неоптимизирован.
	
	etc. :)

-----------------------------------------------------------------------

	Вместе с программой поставляется файл autoexec.cmd, который 
        автоматически исполняется при запуске интерпретатора. Его 
        можно удалить при ненадобности.

	Для вызова примера исполняемого скрипра наберите /example

	Всвязи с молодостью и частичной недоработанности самой 
	программы возможны ошибки в её работе, о которых просьба 
        сообщать на dmitry_gt@tut.by

	В ближайшее время не планируется новых нововведений в консоль, 
        т.к. на её доработку уходит много времени и я считаю, что она 
        и на данный момент более-менее функциональна (в масштабах 
        MenuetOS, конечно). Я считаю, что рациональнее будет сконцент-
        рироваться на устанении багов и недоработок в программе.

	Если кто-нибудь пожелает доделать программу, то я бы попросил
	не изменять её дизайн (мне он нравится ;-) ).

	О всех ошибках (В том числе грамматических в английском тексте)
 	просьба сообщать на dmitry_gt@tut.by

	И еще забыл сказать, то программа распостраняется "as is", и
	автор не несет ответственности за возможный ущерб, причиненный
	программой.

	19.06.2004,
	Chemist


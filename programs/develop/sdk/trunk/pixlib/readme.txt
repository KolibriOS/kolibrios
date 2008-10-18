
typedef unsigned int u32_t;
typedef unsignet int handle;
typedef unsigned int color_t;


handle CreateHatchBrush(int hatch, color_t bkcolor, color_t fcolor);

Создать штрихованную кисть размером 8х8 пикселей

hatch     тип штриховки:

          HS_HORIZONTAL   -------
          HS_VERTICAL     |||||||
          HS_FDIAGONAL    \\\\\\\
          HS_BDIAGONAL    ///////
          HS_CROSS        +++++++
          HS_DIAGCROSS    xxxxxxx

bkcolor   цвет "0"

fcolor    цвет "1"


Возвращаемое значение: логический номер кисти или 0




handle CreateMonoBrush(color_t bkcolor, color_t fcolor,
                         u32_t bmp0, u32_t bmp1);

Создать монохромную кисть размером 8х8 пикселей

bkcolor     цвет "0"

fcolor      цвет "1"

bmp0 bmp1   монохромный битмап 8х8 пикселей


Возвращаемое значение: логический номер кисти или 0



void     DestroyBrush(handle brush);

Уничтожить кисть.

brush     логический номер кисти.


Кисть должна быть создана вызовом CreateHatchBrush или CreateMonoBrush




handle CreatePixmap(unsigned width, unsigned height, u32_t format, u32_t flags);

Создать битмап

width    ширина в пикселях. Максимум 2048

height   высота в пикселях. Максимум 2048

format   формат пикселей. Сейчас поддерживается только ARGB32

flags    дополнительные флаги:

         PX_MEM_SYSTEM =  0 битмап в системной памяти
         PX_MEM_LOCAL  =  1 битмап в локальной видеопамяти
         PX_MEM_GART   =  2 зарезервировано
         остальные биты зарезервированы и должны быть 0


Возвращаемое значение:  логический номер битмапа в случае успеха или 0




int DestroyPixmap( handle pixmap)

Уничтожить битмап.

pixmap   логический номер битмапа.

Битмап должен быть создан CreatePixmap.


Возвращаемое значение: ERR_OK в случае успеха или ERR_PARAM в случае неудачи.




int ClearPixmap(handle pixmap, color_t color)

Заполняет битмап указанным цветом;

pixmap       логический номер битмапа. SCR_PIXMAP для первичного экрана.

color        цвет в формате ARGB32


Возвращаемое значение: ERR_OK в случае успеха или ERR_PARAM в случае неудачи.




int Line(handle pixmap, int x0, int y0, int x1, int y1, color_t color)

Нарисовать сплошную линию указаного цвета толщиной в 1 пиксель.

pixmap       логический номер битмапа в который будет производится отрисовка.
             SCR_PIXMAP для первичного экрана

x0,y0 x1,y1  координаты начальной и конечной точек линиии

color        цвет в формате ARGB32


Возвращаемое значение: ERR_OK в случае успеха или ERR_PARAM в случае неудачи.




int DrawRect(handle pixmap, int xorg, int yorg,
             int width, int height,
             color_t dst_color, color_t border)

Нарисовать сплошной прямоугльник указаного цвета c окантовкой.

pixmap     логический номер битмапа в который будет производится отрисовка.
           SCR_PIXMAP для первичного экрана

xorg,yorg  координаты левого верхнего угла прямоугольника в пикселях

width      ширина прямоугольника в пикселях

height     высота прямоугольника в пикселях

color      цвет прямоугольника в формате ARGB32

border     цвет окантовки в формате ARGB32. Толщина окантовки 1 пиксел.


Возвращаемое значение: ERR_OK в случае успеха или ERR_PARAM в случае неудачи.




int FillRect(handle pixmap, int xorg, int yorg,
             int width, int height,
             brush_t *brush, color_t border)

Нарисовать прямоугольник используя кисть

pixmap     логический номер битмапа в который будет производится отрисовка.
           SCR_PIXMAP для первичного экрана

xorg,yorg  координаты левого верхнего угла прямоугольника в пикселях

width      ширина прямоугольника в пикселях

height     высота прямоугольника в пикселях

brush      монохромная кисть размером 8х8 пикселей

border     цвет окантовки в формате ARGB32. Толщина окантовки 1 пиксел.
           Окантовка не рисуется если альфа компонен цвета равен 0.


Кисть должна быть создана CreateHatch или CreateMonoBrush.


Возвращаемое значение: ERR_OK в случае успеха или ERR_PARAM в случае неудачи.




int Blit(handle dst_pixmap, int dst_x, int dst_y,
         handle src_pixmap, int src_x, int src_y,
         int width, int height)

Скопировать прямоугольную область пикселей.

dst_pixmap    логический номер битмапа в который будет производитс
              копирование. SCR_PIXMAP для первичного экрана.

dst_x, dst_y  координаты левого верхнего угла области назначени

src_pixmap    логический номер битмапа - источника пикселей.
              SCR_PIXMAP для первичного экрана.

src_x,src_y   координаты левого верхнего угла копируемой области

width         ширина копируемой области

height        высота копируемой области

Возвращаемое значение: ERR_OK в случае успеха или ERR_PARAM в случае неудачи.




int TransparentBlit(handle dst_pixmap, int dst_x, int dst_y,
         handle src_pixmap, int src_x, int src_y,
         int width, int height, color_t key)

Скопировать прямоугольную область пикселей используя прозрачный цвет.

dst_pixmap    логический номер битмапа в который будет производитс
              копирование. SCR_PIXMAP для первичного экрана.

dst_x, dst_y  координаты левого верхнего угла области назначени

src_pixmap    логический номер битмапа - источника пикселей.
              SCR_PIXMAP для первичного экрана.

src_x,src_y   координаты левого верхнего угла копируемой области

width         ширина копируемой области

height        высота копируемой области

key           прозрачный цвет в формате ARGB32


Функция не копирует пиксели цвет которых совпадает с key.


Возвращаемое значение: ERR_OK в случае успеха или ERR_PARAM в случае неудачи.

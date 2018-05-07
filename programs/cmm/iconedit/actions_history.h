
#define MAX_ACTIONS_COUNT 15

struct _ActionsHistory {
	dword stack[MAX_ACTIONS_COUNT];
	dword head;
	dword tail;
	dword currentIndex;

	void init();
	bool isEmpty();

	void saveCurrentState();
	void restoreState(dword index);

	void undoLastAction();
	void redoLastAction();
};

void _ActionsHistory::init() {
	dword i;

	head = tail = 0;
	currentIndex = -1;

	for (i = 0; i < MAX_ACTIONS_COUNT; i++) {
		stack[i] = free(stack[i]);
		stack[i] = malloc(image.columns * image.rows * 4);
	}

	saveCurrentState();
}

bool _ActionsHistory::isEmpty() {
	if (head == tail)
		return true;
	else
		return false;
}

void _ActionsHistory::saveCurrentState() {
	dword addr, offset;
	int r, c;
	
	tail = currentIndex + 1;

	if (tail >= MAX_ACTIONS_COUNT)
		tail = tail % MAX_ACTIONS_COUNT;

	addr = stack[tail];

	for (r = 0; r < image.rows; r++)
	{
		for (c = 0; c < image.columns; c++)
		{
			offset = calc(image.columns * r + c) * 4;

			ESDWORD[addr + offset] = image.get_pixel(r, c);
		}
	}

	currentIndex = tail;
	tail = calc(tail + 1) % 10;

	if (tail == head)
		head = calc(head + 1) % 10;
}

void _ActionsHistory::restoreState(dword index) {
	dword addr, offset;
	int r, c;

	addr = stack[index];

	for (r = 0; r < image.rows; r++)
	{
		for (c = 0; c < image.columns; c++)
		{
			offset = calc(image.columns * r + c) * 4;
			image.set_pixel(r, c, ESDWORD[addr + offset]);
		}
	}
}

void _ActionsHistory::undoLastAction() {
	dword previousAction;

	if (!is_selection_moving()) {
		// Если вышли за левую границу, перемещаемся в конец массива
		if (currentIndex == 0) {
			previousAction = MAX_ACTIONS_COUNT - 1;
		}
		else {
			previousAction = currentIndex - 1;
		}

		if (isEmpty())
			return;
		else {
			if (currentIndex != head) {
				restoreState(previousAction);
				DrawCanvas();
			}

			if (currentIndex != head)
				currentIndex = previousAction;
		}
	}
}

void _ActionsHistory::redoLastAction() {
	dword nextAction = calc(currentIndex + 1);

	if (!is_selection_moving()) {
		// Если вышли за левую границу, возвращаемся в начало	
		if (nextAction >= MAX_ACTIONS_COUNT)
			nextAction = nextAction % MAX_ACTIONS_COUNT;

		if (isEmpty())
			return;
		else {
			if (nextAction != tail) {
				restoreState(nextAction);
				DrawCanvas();
			}

			if (nextAction != tail)
				currentIndex = nextAction;
		}
	}
}
int a;

static int b;

static int g() {
	return b;
}

int f() {
	return a + g();
}

int main() {
	return f();
}

const float sctable[91]={0.000,0.017,0.035,0.052,0.069,0.087,0.104,0.121,0.139,0.156,0.173,
			       0.190,0.207,0.225,0.242,0.259,0.275,0.292,0.309,0.325,0.342,
			       0.358,0.374,0.390,0.406,0.422,0.438,0.454,0.469,0.485,0.500,
			       0.515,0.530,0.545,0.560,0.573,0.588,0.602,0.616,0.629,0.643,
			       0.656,0.669,0.682,0.695,0.707,0.719,0.731,0.743,0.754,0.766,
			       0.777,0.788,0.798,0.809,0.819,0.829,0.838,0.848,0.857,0.866,
			       0.875,0.883,0.891,0.899,0.906,0.913,0.920,0.927,0.934,0.939,
			       0.945,0.951,0.956,0.961,0.965,0.970,0.974,0.978,0.982,0.985,
			       0.987,0.990,0.992,0.994,0.996,0.997,0.998,0.999,0.999,1.000};

float sin (int angle) {
	float res=1;
	angle%=360;
	if (angle>180) res=-1;
	if (angle>=0 && angle<=90) return (res*sctable[angle]);
	if (angle>90 && angle<=180) return (res*sctable[180-angle]);
	if (angle>180 && angle<=270) return (res*sctable[angle-180]);
	if (angle>270) return (res*sctable[360-angle]);
	return res;
}

float cos (int angle) {
	float res=1;
	angle%=360;
	if (angle>90 && angle<270) res=-1;
	if (angle>=0 && angle<=90) return (res*sctable[90-angle]);
	if (angle>90 && angle<=180) return (res*sctable[angle-90]);
	if (angle>180 && angle<=270) return (res*sctable[90-(angle-180)]);
	if (angle>270) return (res*sctable[90-(360-angle)]);
	return res;
}

int FloatToInt (float a) {
	int sign=1;
	int result=0;
	if (a<1 && a>(-1)) return 0;
	if (a<0) {
		sign=-1;
		a*=(-1);
	}
	while (a>=1) {
		a--;
		result++;
	}
	return (result*sign);
}

int min (int a, int b) {
	if (a<=b) return a;
	return b;
}

int max (int a, int b) {
	if (a>=b) return a;
	return b;
}

void IntToStr (int Value, char Str[]) {
	char Stack[100]="";
	int StackLen=0;
	if (Value==0) {
		Str[0]='0';
		Str[1]='\0';
		return;
	}
	while (Value!=0) {
		Stack[StackLen]=Value%10+48;
		Value/=10;
		StackLen++;
	}
	int i=0;
	for (i=0; i<StackLen; i++) {
		Str[i]=Stack[StackLen-i-1];
	}
	Str[i]='\0';
}

int Abs (int a) {
	if (a<0) return (-1)*a;
	return a;
}



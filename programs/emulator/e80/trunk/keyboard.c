
///=============================

void keyboard_process(unsigned key)
{

switch (key)
	{

	case 42: // L-Shift Down
	case 54: // R-Shift Down
		fila[4][1] &= 0xFE;
		break;

	case 42+128: // L-Shift Up
	case 54+128: // R-Shift Up
		fila[4][1] |= 1;
		break;

	case 29: // Ctrl Down
		fila[4][2] &= 0xFD;
		break;

	case 29+128: // Ctrl Up
		fila[4][2] |= 2;
		break;

	case 56: // Alt Down
		fila[4][1] &= 0xFE;
		fila[4][2] &= 0xFD;
		break;

	case 56+128: // Alt Up
		fila[4][1] |= 1;
		fila[4][2] |= 2;
		break;

	case 28: // Enter Down
		fila[3][2] &= 0xFE;
		break;

	case 28+128: // Enter Up
		fila[3][2] |= 1;
		break;


	case 2: // 1 Down
		fila[1][1] &= 0xFE;
		break;

	case 2+128: // 1 Up
		fila[1][1] |= 1;
		break;

	case 3: // 2 Down
		fila[1][1] &= 0xFD;
		break;

	case 3+128: // 2 Up
		fila[1][1] |= 2;
		break;

	case 4: // 3 Down
		fila[1][1] &= 0xFB;
		break;

	case 4+128: // 3 Up
		fila[1][1] |= 4;
		break;

	case 5: // 4 Down
		fila[1][1] &= 0xF7;
		break;

	case 5+128: // 4 Up
		fila[1][1] |= 8;
		break;

	case 6: // 5 Down
		fila[1][1] &= 0xEF;
		break;

	case 6+128: // 5 Up
		fila[1][1] |= 16;
		break;

	case 7: // 6 Down
		fila[1][2] &= 0xEF;
		break;

	case 7+128: // 6 Up
		fila[1][2] |= 16;
		break;

	case 8: // 7 Down
		fila[1][2] &= 0xF7;
		break;

	case 8+128: // 7 Up
		fila[1][2] |= 8;
		break;

	case 9: // 8 Down
		fila[1][2] &= 0xFB;
		break;

	case 9+128: // 8 Up
		fila[1][2] |= 4;
		break;

	case 10: // 9 Down
		fila[1][2] &= 0xFD;
		break;

	case 10+128: // 9 Up
		fila[1][2] |= 2;
		break;

	case 11: // 0 Down
		fila[1][2] &= 0xFE;
		break;

	case 11+128: // 0 Up
		fila[1][2] |= 1;
		break;

	case 16: // Q Down
		fila[2][1] &= 0xFE;
		break;

	case 16+128: // Q Up
		fila[2][1] |= 1; 
		break;

	case 17: // W Down
		fila[2][1] &= 0xFD;
		break;

	case 17+128: // W Up
		fila[2][1] |= 2; 
		break;

	case 18: // E Down
		fila[2][1] &= 0xFB;
		break;

	case 18+128: // E Up
		fila[2][1] |= 4; 
		break;

	case 19: // R Down
		fila[2][1] &= 0xF7;
		break;

	case 19+128: // R Up
		fila[2][1] |= 8; 
		break;

	case 20: // T Down
		fila[2][1] &= 0xEF;
		break;

	case 20+128: // T Up
		fila[2][1] |= 16;
		break;

	case 21: // Y Down
		fila[2][2] &= 0xEF;
		break;

	case 21+128: // Y Up
		fila[2][2] |= 16;
		break;

	case 22: // U Down
		fila[2][2] &= 0xF7;
		break;

	case 22+128: // U Up
		fila[2][2] |= 8;
		break;

	case 23: // I Down
		fila[2][2] &= 0xFB;
		break;

	case 23+128: // I Up
		fila[2][2] |= 4;
		break;

	case 24: // O Down
		fila[2][2] &= 0xFD;
		break;

	case 24+128: // O Up
		fila[2][2] |= 2;
		break;

	case 25: // P Down
		fila[2][2] &= 0xFE;
		break;

	case 25+128: // P Up
		fila[2][2] |= 1;
		break;

	case 30: // A Down
		fila[3][1] &= 0xFE;
		break;

	case 30+128: // A Up
		fila[3][1] |= 1;
		break;

	case 31: // S Down
		fila[3][1] &= 0xFD;
		break;

	case 31+128: // S Up
		fila[3][1] |= 2;
		break;

	case 32: // D Down
		fila[3][1] &= 0xFB;
		break;

	case 32+128: // D Up
		fila[3][1] |= 4;
		break;

	case 33: // F Down
		fila[3][1] &= 0xF7;
		break;

	case 33+128: // F Up
		fila[3][1] |= 8;
		break;

	case 34: // G Down
		fila[3][1] &= 0xEF;
		break;

	case 34+128: // G Up
		fila[3][1] |= 16;
		break;

	case 35: // H Down
		fila[3][2] &= 0xEF;
		break;

	case 35+128: // H Up
		fila[3][2] |= 16;
		break;

	case 36: // J Down
		fila[3][2] &= 0xF7;
		break;

	case 36+128: // J Up
		fila[3][2] |= 8;
		break;

	case 37: // K Down
		fila[3][2] &= 0xFB;
		break;

	case 37+128: // K Up
		fila[3][2] |= 4;
		break;

	case 38: // L Down
		fila[3][2] &= 0xFD;
		break;

	case 38+128: // L Up
		fila[3][2] |= 2;
		break;

	case 44: // Z Down
		fila[4][1] &= 0xFD;
		break;

	case 44+128: // Z Up
		fila[4][1] |= 2;
		break;

	case 45: // X Down
		fila[4][1] &= 0xFB;
		break;

	case 45+128: // X Up
		fila[4][1] |= 4;
		break;

	case 46: // C Down
		fila[4][1] &= 0xF7;
		break;

	case 46+128: // C Up
		fila[4][1] |= 8;
		break;

	case 47: // V Down
		fila[4][1] &= 0xEF;
		break;

	case 47+128: // V Up
		fila[4][1] |= 16;
		break;

	case 48: // B Down
		fila[4][2] &= 0xEF;
		break;

	case 48+128: // B Up
		fila[4][2] |= 16;
		break;

	case 49: // N Down
		fila[4][2] &= 0xF7;
		break;

	case 49+128: // N Up
		fila[4][2] |= 8;
		break;

	case 50: // M Down
		fila[4][2] &= 0xFB;
		break;

	case 50+128: // M Up
		fila[4][2] |= 4;
		break;

	case 57: // Space Down
		fila[4][2] &= 0xFE;
		break;

	case 57+128: // Space Up
		fila[4][2] |= 1;
		break;

	case 14: // Backspace Down
		fila[1][2] &= 0xFE;
		fila[4][1] &= 0xFE;
		break;

	case 14+128: // Backspace Up
		fila[1][2] |= 1;
		fila[4][1] |= 1;
		break;

	case 12: // - Down
		fila[3][2] &= 0xF7;
		fila[4][2] &= 0xFD;
		break;

	case 12+128: // - Up
		fila[3][2] |= 8;
		fila[4][2] |= 2;
		break;

	case 53: // / Down
		fila[4][1] &= 0xEF;
		fila[4][2] &= 0xFD;
		break;

	case 53+128: // / Up
		fila[4][1] |= 16;
		fila[4][2] |= 2;
		break;

	case 52: // . Down
		fila[4][2] &= 0xFB;
		fila[4][2] &= 0xFD;
		break;

	case 52+128: // . Up
		fila[4][2] |= 4;
		fila[4][2] |= 2;
		break;

	case 51: // , Down
		fila[4][2] &= 0xF7;
		fila[4][2] &= 0xFD;
		break;

	case 51+128: // , Up
		fila[4][2] |= 8;
		fila[4][2] |= 2;
		break;

	case 13: // = Down
		fila[3][2] &= 0xFD;
		fila[4][2] &= 0xFD;
		break;

	case 13+128: // = Up
		fila[3][2] |= 2;
		fila[4][2] |= 2;
		break;

	};

}

///=============================


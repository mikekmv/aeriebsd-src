
function pentag(X, Y, d, a, x, y)
{
	x = X;
	y = Y;

	x = X + d * cos(a);
	y = Y + d * sin(a);
	printf("l%c%c%c%c%c%c%c%c",
	    int(X % 256), int(X / 256), int(Y % 256), int(Y / 256),
	    int(x % 256), int(x / 256), int(y % 256), int(y / 256));
	a += 2 * 3.1415926 / 5;

	x = X + d * cos(a);
	y = Y + d * sin(a);
	printf("n%c%c%c%c",
	    int(x % 256), int(x / 256), int(y % 256), int(y / 256));
	a += 2 * 3.1415926 / 5;

	x = X + d * cos(a);
	y = Y + d * sin(a);
	printf("n%c%c%c%c",
	    int(x % 256), int(x / 256), int(y % 256), int(y / 256));
	a += 2 * 3.1415926 / 5;

	x = X + d * cos(a);
	y = Y + d * sin(a);
	printf("n%c%c%c%c",
	    int(x % 256), int(x / 256), int(y % 256), int(y / 256));
	a += 2 * 3.1415926 / 5;

	x = X + d * cos(a);
	y = Y + d * sin(a);
	printf("n%c%c%c%c",
	    int(x % 256), int(x / 256), int(y % 256), int(y / 256));
	a += 2 * 3.1415926 / 5;
}

BEGIN {
	D = 20;
	N = 72;
	S = 5;
	W = 4 * D + S * N;
	H = 4 * D + 2 * sqrt(W);
	printf("es%c%c%c%c%c%c%c%c", 0, 0, 0, 0,
	    int(W % 256), int(W / 256), int(H % 256), int(H / 256));
	x = 2 * D;
	for (a = 0; a < N; a++) {
		pentag(x, 2 * D + 2 * sqrt(x), D, a * 3.1415926 / 180);
		x += S;
	}
}

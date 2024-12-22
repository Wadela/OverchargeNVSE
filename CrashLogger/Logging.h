#pragma once

static int logDepth = 0;

class Scope {
public:
	Scope() {
		logDepth++;
	}

	~Scope() {
		logDepth--;
	}
};

#define CONCAT(a, b) CONCAT_INNER(a, b)
#define CONCAT_INNER(a, b) a ## b
#define BEGIN_SCOPE() auto CONCAT(base, __COUNTER__) = Scope{}

void log(const char* fmt, ...) {
	for (int i = 0; i < logDepth; i++) {
		if (i == logDepth - 1) {
			printf("%s", "* ");
		}
		else {
			printf("%s", "  ");
		}
	}

	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	printf("\n");
}
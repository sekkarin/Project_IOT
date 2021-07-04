
// It is not recommended to put function definitions
// in a header file. Ideally there should be only
// function declarations. Purpose of this code is
// to only demonstrate working of header files.

void add(int a, int b)
{
    // Serial.begin(115200);
    Serial.printf_P("Added value=%d\n", a + b);
}

void multiply(int a, int b)
{
    Serial.printf_P("Multiplied value=%d\n", a * b);
}
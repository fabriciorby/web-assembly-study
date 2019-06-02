#include <stdio.h>
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
int sum(int n1, int n2)
{
    return n1 + n2;
}
EMSCRIPTEN_KEEPALIVE
int sub(int n1, int n2)
{
    return n1 - n2;
}
EMSCRIPTEN_KEEPALIVE
int mult(int n1, int n2)
{
    return n1 * n2;
}
EMSCRIPTEN_KEEPALIVE
int divide(int n1, int n2)
{
    return n1 / n2;
}

EM_JS(void, two_alerts, (), {
    alert('hai');
    alert('bai');
});

EMSCRIPTEN_KEEPALIVE
int main()
{
    two_alerts();
    emscripten_run_script("alert('I have been called from C!')");
    EM_ASM(
        x = 1;
        console.log(x););
    EM_ASM(
        console.log(x););
}
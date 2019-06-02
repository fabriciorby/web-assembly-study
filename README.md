# WebAssembly

Para ver no browser e conseguir ver o console do Chrome em tempo real: https://fabriciorby.me/web-assembly-study/

## O que é isso?

https://braziljs.org/artigos/iniciando-com-webassembly-parte-1/

## Meus testes

Após **muita dificuldade**, consegui instalar o compilador de C/C++ para .wasm/.js, que foi o [emscripten](https://emscripten.org/index.html).

Depois de instalado, tive **muita dificuldade** para conseguir compilar meu arquivo, seguindo o tutorial do próprio [WebAssembly](https://webassembly.org/getting-started/developers-guide/).

Após conseguir compilar e subir meu servidor http, vi ele rodando no html proposto e então fiquei feliz.

Essa felicidade não durou muito, pois vi que minha página html importava um `.js`, e não um `.wasm`, o que não faz sentido nenhum de acordo com o que eu vi [aqui](https://braziljs.org/artigos/iniciando-com-webassembly-parte-2/).

Creio que por ser uma tecnologia relativamente nova, há muitos artigos escritos que estão desatualizados e, vendo agora, esse foi um dos motivos para a minha frustração nessa jornada.

Assim começa a minha saga para tentar importar um `.wasm` ao invés do `.js`, e foi assim que me deparei com incontáveis erros no meu console do Chrome.

Mas eu consegui!

### Meu código-fonte

```C
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
        console.log(x);
    );
    EM_ASM(
        console.log(x);
    );
}
```

### wasm puro

Após inúmeros testes e pesquisas cheguei no resultado que o caminho mais curto para conseguir utilizá-lo em seu browser é compilá-lo da seguinte forma: `emcc hello.c -Os -o hello-wasm.wasm`

**Importante**: Consegui compilar minhas funções pois utilizei a notação `EMSCRIPTEN_KEEPALIVE` logo antes das minhas funções, caso contrário o compilador corta fora minhas funções por default. Outra forma de fazer isso é adicionando a flag `EXPORTED_FUNCTIONS`. Nesse caso, se eu não tivesse a notação, minha compilação deveria ser chamada como:
`emcc hello.c -Os -o hello-wasm.wasm EXPORTED_FUNCTIONS='["_main", "_sum", "_sub", "_divide", "_mult"]'`.

Para que funcione, é necessário subir um servidor http e, por sorte, nosso compilador pode fazer isso por nós via `emrun --no_browser --port 8080 .`

A forma mais simples de importá-lo no browser é a seguinte:

```javascript
(async () => {
    const importObject = {
            env: {
            _two_alerts: _ => { },
            _emscripten_asm_const_i: _ => { },
            _emscripten_run_script: _ => { },
            memory: new WebAssembly.Memory({
                initial: 256,
                maximum: 256
            })
        }
    };
    const {instance} = await WebAssembly.instantiateStreaming(fetch('hello-wasm.wasm'), importObject);
    result = instance.exports;
})();
```

Por que essas funções vazias? São funções que estão sendo chamados pelo meu arquivo compilado `.wasm` que eu não possuo definição no momento de compilação, então tenho que gerar esse esqueleto e adicionar alguma coisa, pois senão ele não é instanciado e gera um erro chato do tipo `function import requires a callable`.

Após carregar a página, é só entrar no console do Chrome e mandar os comandos para usar suas funções em WebAssembly:
```
result._mult(5, 5);

25
```

Entretanto se eu tento executar minha `result._main()`, nada do que eu escrevi no meu código-fonte em C ocorre...

Isso é porque essas funções EM_ASM, EM_JS e emscripten_run_script são geradas no arquivo `.js` que vem junto com a compilação e, se eu realmente quiser utilizá-las sem ter que importar o .js, terei que pegar emprestado do `.js` e reescrevê-las no meu `importObject`:

```javascript
var ASM_CONSTS = [function () { x = 1; console.log(x) }, function () { console.log(x) }];
function _emscripten_asm_const_i(code) { return ASM_CONSTS[code]() }
function _two_alerts() { alert("hai"); alert("bai") }
function _emscripten_run_script(ptr) { eval(UTF8ToString(ptr)) }
```

```javascript
(async () => {
    const ASM_CONSTS = [function () { x = 1; console.log(x) }, function () { console.log(x) }];
    const importObject = {
        env: {
            _two_alerts: _ => { alert("hai"); alert("bai") },
            _emscripten_asm_const_i: _ => { ASM_CONSTS[_]() },
            _emscripten_run_script: _ => { eval(_) },
            memory: new WebAssembly.Memory({
                initial: 256,
                maximum: 256
            })
        }
    };
    const { instance } = await WebAssembly.instantiateStreaming(fetch('hello-wasm.wasm'), importObject);
    result = instance.exports;
})();
```

Pronto! Apesar de não ser **nada prático**, está funcionando da forma como deveria estar.

### Importando via `.js`

Para compilá-lo via `.js`, utilizei o seguinte comando: `emcc hello.c -Os -o hello.js -s WASM=1`

A forma mais simples de importar ao browser é a seguinte:
`<script async src="hello.js"></script>`

Assim que a tela for carregada será gerado um objeto global chamado `Module`, e o `_main()` será executado.

Para ter acesso às outras funções do código-fonte em C, entre no console do Chrome e digite:
```
Module.mult(5, 5);

25
```
Pronto! Seu WebAssembly está funcionando com um `.js` auxiliar.

## Conclusão

Há duas formas de utilizar o WebAssembly, importando o .wasm diretamente no browser via `WebAssembly.instantiateStreaming(fetch('hello.wasm'), importObject);` e a mais simples e útil, importando o .js com `<script async src="hello.js"></script>`.

Para fazer chamadas do código-fonte em C via `EM_JS`/`EM_ASM`/`emscripten_run_script()` pelo `.wasm` puro, é necessário auxílio do `.js`, nem que seja para pegar as funções "emprestadas".

Para algo mais robusto em WebAssembly, por favor utilize o .js compilado.

No momento o desenvolvimento de WebAssembly está muito complicada e, a não ser que haja uma tarefa feita pelo navegador que **realmente** seja necessária um aumento no tempo de processamento, o WebAssembly não atende.

Atualmente, a dificuldade para instalação, compilação e codificação não valem a pena para o mercado. Até para pesquisar sobre os erros que eu estava tomando foi **muito** complicado.

Além disso, para você ter uma ideia, a pasta do meu compilador, após instalação, está com **1,85GB** de espaço. Ok que talvez eu não esteja com as configurações mais ótimas para o desenvolvimento, entretanto, mesmo assim é muita coisa para um compilador.

### Links úteis

Links úteis já que a documentação não é muito gentil
- https://ariya.io/2019/05/basics-of-memory-access-in-webassembly
- https://agniva.me/wasm/2018/05/17/wasm-hard-way.html
- https://marcoselvatici.github.io/WASM_tutorial/index.html

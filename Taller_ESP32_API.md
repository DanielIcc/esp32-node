# Taller básico: el ESP32 y una API en la red local

Un servidor en Node.js guarda si un LED debe estar encendido. El ESP32, en el mismo WiFi, pregunta ese dato cada segundo y enciende o apaga su LED.

No hace falta instalar librerías. Node trae el módulo `http`.

Cada persona corre el servidor en su computadora. El ESP32 apunta a la IP de esa computadora. No usa internet ni una nube.

---

## 1. Qué vas a armar

```text
Navegador  --encender/apagar-->  Node.js en tu PC  <--pregunta--  ESP32
                                 puerto 3000                      LED del GPIO 2
```

Rutas:

| Método | Ruta | Quién la usa | Qué hace |
| --- | --- | --- | --- |
| GET | `/` | Tú, en el navegador | Muestra el estado y dos enlaces |
| GET | `/encender` | Tú | Guarda el LED en encendido |
| GET | `/apagar` | Tú | Guarda el LED en apagado |
| GET | `/comando` | El ESP32 | Responde `{"led":true}` o `{"led":false}` |

---

## 2. Materiales

- Node.js LTS, desde [https://nodejs.org](https://nodejs.org). Al terminar, `node -v` debe mostrar un número de versión.
- Placa ESP32 con WiFi (DevKit de 30 o 38 pines) y un cable USB de datos.
- Arduino IDE 2, con el soporte de placas ESP32.
- La computadora y el ESP32 en la misma red. El ESP32 solo entra a WiFi de **2.4 GHz**.

La computadora puede ir por cable al router. No hace falta que ella use WiFi.

En Arduino IDE:

1. **Archivo → Preferencias → Gestor de URLs Adicionales de Tarjetas** y añade:

   ```text
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```

2. **Herramientas → Placa → Gestor de tarjetas**, busca `esp32` de Espressif e instálalo.
3. Elige la placa **ESP32 Dev Module** y el puerto COM de tu placa.

`WiFi.h` y `HTTPClient.h` vienen con ese paquete.

---

## 3. Cuatro ideas

**API.** Un acuerdo para que dos programas se hablen. Aquí el acuerdo son unas rutas HTTP en tu computadora.

**GET.** Pedir algo. Abrir una dirección en el navegador es un GET. El ESP32 hace el mismo tipo de petición, a `/comando`.

**JSON.** Texto con datos. La respuesta del servidor se ve así:

```json
{"led": true}
```

`true` es encendido. `false` es apagado.

**IP local.** La dirección de tu computadora dentro de la red. En Windows:

```bat
ipconfig
```

Anota la **Dirección IPv4** del Wi-Fi o del Ethernet. Se parece a `192.168.1.20`.

| Dirección | Significado |
| --- | --- |
| `127.0.0.1` | Esta misma máquina. En el ESP32 es el propio ESP32, no tu PC. |
| `0.0.0.0` | Al arrancar el servidor: escucha por todas las redes de la PC, para que el ESP32 pueda entrar. |
| La IPv4 de `ipconfig` | La que escribes en el ESP32 y en el navegador del teléfono. |

Una URL de este taller:

```text
http://192.168.1.20:3000/comando
       IP de tu PC      puerto  ruta
```

---

## 4. El servidor

Archivo: `codigo/servidor.js`.

Node crea un servidor. Cada vez que llega una petición, corre la función que le pasaste a `createServer`. `req.method` es el método (`GET`). `req.url` es la ruta (`/comando`). `res.end` envía la respuesta y cierra.

`JSON.stringify({ led: led })` convierte la variable en el texto `{"led":true}` o `{"led":false}`.

`/encender` y `/apagar` cambian la variable y responden con un código 302. El navegador entiende ese código como «ahora abre `/`», y por eso vuelves a ver la página ya actualizada.

```javascript
const http = require("http");

let led = false;

const server = http.createServer((req, res) => {
  if (req.method === "GET" && req.url === "/comando") {
    res.writeHead(200, { "Content-Type": "application/json" });
    res.end(JSON.stringify({ led: led }));
    return;
  }

  if (req.method === "GET" && req.url === "/encender") {
    led = true;
    res.writeHead(302, { Location: "/" });
    res.end();
    return;
  }

  if (req.method === "GET" && req.url === "/apagar") {
    led = false;
    res.writeHead(302, { Location: "/" });
    res.end();
    return;
  }

  if (req.method === "GET" && req.url === "/") {
    const texto = led ? "encendido" : "apagado";
    res.writeHead(200, { "Content-Type": "text/html; charset=utf-8" });
    res.end(
      "<h1>LED: " + texto + "</h1>" +
        '<p><a href="/encender">Encender</a></p>' +
        '<p><a href="/apagar">Apagar</a></p>'
    );
    return;
  }

  res.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" });
  res.end("Ruta no existe");
});

server.listen(3000, "0.0.0.0", () => {
  console.log("Servidor listo en el puerto 3000");
});
```

`listen(3000, "0.0.0.0")` abre el puerto 3000 para otros equipos de la red. Si escucharas solo en `127.0.0.1`, el ESP32 no podría entrar.

Arranca, dentro de la carpeta `codigo`:

```bash
node servidor.js
```

Deja esa terminal abierta. Para parar: Ctrl+C. Si cambias el archivo, para y vuelve a ejecutar.

Si dice que el puerto está en uso (`EADDRINUSE`), ya hay otro `node servidor.js` corriendo. Ciérralo, o cambia `3000` aquí y también en el ESP32.

---

## 5. Probarlo en el navegador

En la misma computadora abre:

```text
http://127.0.0.1:3000/
```

Pulsa **Encender** y **Apagar**. El título cambia.

Abre también:

```text
http://127.0.0.1:3000/comando
```

Ahí está el JSON que va a leer el ESP32. Con el LED encendido se ve `{"led":true}`.

Después, en el teléfono conectado al mismo WiFi, abre `http://IP_DE_LA_PC:3000/` usando la IPv4 de `ipconfig`. Si el teléfono entra, la red está lista.

Si Windows pregunta por el firewall, permite el acceso en redes privadas.

---

## 6. El ESP32

Archivo: `codigo/esp32_api/esp32_api.ino`. La carpeta y el archivo deben llamarse igual.

Cambia el nombre de la red, la contraseña y la IP. La URL lleva `http://`, la IP de tu PC, el puerto `3000` y la ruta `/comando`.

```cpp
const char* WIFI_SSID = "TU_RED";
const char* WIFI_PASS = "TU_CONTRASENA";
const char* URL_COMANDO = "http://192.168.1.20:3000/comando";
```

El programa se conecta al WiFi y, cada segundo, hace un GET. Si la respuesta trae la palabra `true`, enciende el GPIO 2. En muchas placas DevKit ese pin es el LED de la tarjeta.

Programa completo:

```cpp
#include <WiFi.h>
#include <HTTPClient.h>

const char* WIFI_SSID = "TU_RED";
const char* WIFI_PASS = "TU_CONTRASENA";
const char* URL_COMANDO = "http://192.168.1.20:3000/comando";

const int LED_PIN = 2;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Conectando");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("ESP32 en ");
  Serial.println(WiFi.localIP());
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    return;
  }

  WiFiClient cliente;
  HTTPClient http;
  if (!http.begin(cliente, URL_COMANDO)) {
    Serial.println("No se pudo preparar la peticion");
    delay(1000);
    return;
  }

  int codigo = http.GET();
  if (codigo == 200) {
    String cuerpo = http.getString();
    Serial.println(cuerpo);
    bool encender = cuerpo.indexOf("true") >= 0;
    digitalWrite(LED_PIN, encender ? HIGH : LOW);
  } else {
    Serial.printf("Fallo: %d\n", codigo);
  }

  http.end();
  delay(1000);
}
```

Súbelo desde Arduino IDE. Abre el **Monitor serie** a **115200** baudios.

Si la carga falla al final, mantén **BOOT** al empezar la subida y suéltalo cuando aparezca el porcentaje.

---

## 7. Juntar las dos partes

1. Anota la IPv4 de la computadora.
2. Ejecuta `node servidor.js` y déjalo abierto.
3. Prueba `http://127.0.0.1:3000/comando` en la PC.
4. Escribe esa IP en `URL_COMANDO`.
5. Sube el programa al ESP32.

En el monitor serie, más o menos:

```text
Conectando......
ESP32 en 192.168.1.54
{"led":false}
{"led":true}
```

Pulsa **Encender** en el navegador. En el siguiente segundo el LED de la placa se enciende. **Apagar** lo apaga.

Durante el arranque el LED puede parpadear solo. Después debe seguir lo que diga `/comando`.

---

## 8. Si algo falla

| Lo que ves | Qué revisar |
| --- | --- |
| Se queda en «Conectando» | Nombre del WiFi, contraseña, y que la red sea de 2.4 GHz. |
| `Fallo: -1` | El ESP32 no llegó al servidor. Revisa la IP, que `node servidor.js` siga abierto, el puerto 3000 y el firewall. |
| La PC abre la página y el teléfono no | En el teléfono, `127.0.0.1` es el teléfono. Usa la IP de `ipconfig`. Algunos hotspots de celular no dejan que un aparato hable con otro. |
| `Fallo: 404` | La ruta no es `/comando`, o sobra una barra o un espacio en la URL. |
| El serie muestra `{"led":true}` y el LED no cambia | El LED de tu placa no está en el GPIO 2, o se enciende con `LOW`. Invierte `HIGH` y `LOW` en el `digitalWrite`. |
| `EADDRINUSE` | El puerto 3000 ya está ocupado. Cierra el otro Node con Ctrl+C. |

Esta API no tiene contraseña. Úsala en la red de casa o del taller. No abras el puerto 3000 hacia internet.

---

## 9. Un paso más: el ESP32 envía un número

Cuando el LED ya obedece, puedes dejar que la placa también **entregue** un dato. Eso es un POST.

En `servidor.js`, antes de la respuesta 404, añade:

```javascript
if (req.method === "POST" && req.url === "/datos") {
  let texto = "";
  req.on("data", (parte) => {
    texto += parte;
  });
  req.on("end", () => {
    const cuerpo = JSON.parse(texto);
    console.log("Dato recibido:", cuerpo.valor);
    res.writeHead(200, { "Content-Type": "application/json" });
    res.end(JSON.stringify({ ok: true }));
  });
  return;
}
```

Para y vuelve a arrancar el servidor. En el `loop()` del ESP32, después de leer el LED, puedes mandar un contador:

```cpp
static int contador = 0;
contador++;

WiFiClient clientePost;
HTTPClient post;
post.begin(clientePost, "http://192.168.1.20:3000/datos");
post.addHeader("Content-Type", "application/json");
String cuerpo = "{\"valor\":" + String(contador) + "}";
int codigoPost = post.POST(cuerpo);
Serial.printf("POST %s -> %d\n", cuerpo.c_str(), codigoPost);
post.end();
```

Cambia la IP de esa URL por la de tu computadora. En la terminal de Node debe aparecer `Dato recibido: 1`, luego `2`, y así cada segundo.

`GET` pide. `POST` entrega. Las dos peticiones hablan con el mismo programa en tu red local.

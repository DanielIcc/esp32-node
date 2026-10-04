const http = require("http");

let led = false;

const server = http.createServer((request, response) => {
  
  if (request.method === "GET" && request.url === "/comando") {
    response.writeHead(200, { "Content-Type": "application/json" });
    response.end(JSON.stringify({ led: led }));
    return;
  }

  if (request.method === "GET" && request.url === "/encender") {
    led = true;
    response.writeHead(302, { Location: "/" });
    response.end();
    return;
  }

  if (request.method === "GET" && request.url === "/apagar") {
    led = false;
    response.writeHead(302, { Location: "/" });
    response.end();
    return;
  }

  if (request.method === "GET" && request.url === "/") {
    const texto = led ? "encendido" : "apagado";
    response.writeHead(200, { "Content-Type": "text/html; charset=utf-8" });
    response.end(
      "<h1>LED: " + texto + "</h1>" +
        '<p><a href="/encender">Encender</a></p>' +
        '<p><a href="/apagar">Apagar</a></p>'
    );
    return;
  }

  response.writeHead(404, { "Content-Type": "text/plain; charset=utf-8" });
  response.end("Ruta no existe");
});

server.listen(3000, () => {
  console.log("Servidor listo en el puerto 3000");
});
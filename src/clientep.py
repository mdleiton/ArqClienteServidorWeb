import requests
import json
headers= {"Content-Type":"application/json"}

#CREAMOS UN TIPO DE SOLICITUD CON METODO NO DEFINIDO EN EL SERVIDOR COMO:PUT , DELETE, OPTIONS , HEADERS
r = requests.put("http://127.0.0.1:8889/")
print("ENCABEZADOS:",r.headers)
print("Codigo estado",r.status_code)
print("json respuesta:",r.json())
print("")

# Creamos la peticion HTTP con GET con url incorrecta:
r = requests.get("http://127.0.0.1:8889/sf")
# Imprimimos el resultado si el cdigo de estado HTTP es 200 (OK):
print("ENCABEZADOS:",r.headers)
print("Codigo estado",r.status_code)
print("json respuesta:",r.json())
print("")

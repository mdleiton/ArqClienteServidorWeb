import requests
# Creamos la peticin HTTP con GET:
r = requests.get("http://127.0.0.1:8888/", params = {"w":"774508"})
# Imprimimos el resultado si el cdigo de estado HTTP es 200 (OK):
if r.status_code == 200:
    #print r.text
    print("good connection")

# Creamos la peticion HTTP tipo post:
print('segundo tipo e solicitud') 
contenido = {'name': 'data'}

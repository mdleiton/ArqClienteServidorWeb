# -*- coding: utf8 -*-.
import requests
import json
import sys 
if sys.argv[3]=="listar_dispositivos":
    if len(sys.argv) == 4 and sys.argv[2]=="GET":
        url="http://127.0.0.1:"+str(sys.argv[1])+"/"+sys.argv[3]
        print(url)
        r=requests.get(url)
        dic=r.json()
        print("ENCABEZADOS:",r.headers)
        print("CODIGO ESTADO:",r.status_code)
        print("JSON RESPUESTA:",dic)
        print("JSON STATUS:",dic['status'])
        print("LISTADO DE DISPOSITIVO:",dic['dispositivos'])
    else:
        print("el nÃºmero de parametros es incorrecto para la solicitud o el metodo para la solicitud es incorrecto")
elif sys.argv[3]=="nombrar_dispositivo":
    if len(sys.argv) == 6 and sys.argv[2]=="POST":
        url="http://127.0.0.1:"+str(sys.argv[1])+"/"+sys.argv[3]
        jsons={"solicitud":sys.argv[3],"nodo": sys.argv[4],"nombre":sys.argv[5]}
        print(url)
        r=requests.post(url,json=json.dumps(jsons))
        dic=r.json()
        print("ENCABEZADOS:",r.headers)
        print("CODIGO ESTADO:",r.status_code)
        print("JSON RESPUESTA:",dic)
        print("JSON STATUS:",dic['status'])
    else:
        print("el nÃºmero de parametros es incorrecto para la solicitud o el metodo para la solicitud es incorrecto")
elif sys.argv[3]=='leer_archivo':
    if len(sys.argv) == 6 and sys.argv[2]=="GET":
        url="http://127.0.0.1:"+str(sys.argv[1])+"/"+sys.argv[3]
        jsons={"solicitud":sys.argv[3],"nombre": sys.argv[4],"nombre_archivo":sys.argv[5]}
        print(url)
        r=requests.get(url,json=jsons)
        dic=r.text
        print("ENCABEZADOS:",r.headers)
        print("CODIGO ESTADO:",r.status_code)
        print("JSON RESPUESTA:",dic)
        #print("JSON STATUS:",dic['status'])
    else:
        print("el nÃºmero de parametros es incorrecto para la solicitud o el metodo para la solicitud es incorrecto")
elif sys.argv[3]=='escribir_archivo':
    if len(sys.argv) == 6 and sys.argv[2]=="POST":
        url="http://127.0.0.1:"+str(sys.argv[1])+"/"+sys.argv[3]
        archivo = open(sys.argv[5])
        contenido=""
        for linea in archivo:
            contenido=contenido+linea
        archivo.close()
        jsons={"solicitud":sys.argv[3],"nombre": sys.argv[4],"nombre_archivo":sys.argv[5],"tamano_contenido":len(contenido),"contenido":contenido}
        print(url)
        r=requests.post(url,json=jsons)
        dic=r.json()
        print("ENCABEZADOS:",r.headers)
        print("CODIGO ESTADO:",r.status_code)
        print("JSON RESPUESTA:",dic)
        print("JSON STATUS:",dic['status'])
    else:
        print("el nÃºmero de parametros es incorrecto para la solicitud o el metodo para la solicitud es incorrecto")
else:
    print("solicitud no definida")


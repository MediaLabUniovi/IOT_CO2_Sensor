# pro-sensor-co2
Sensor de CO2 con indicador LED y conexión WiFi desarrollado por MediaLab UniOvi

![Static Badge](https://img.shields.io/badge/Estabilidad-v1.0-3314333)

# <p align="center"> PRO SENSOR CO2 - MEDIALAB UNIOVI </p>

## Sensor de CO2 para interiores con conectividad WiFi para almacenar la evolución de una sala en la nube e indicadores LED para ver de forma presencial el estado de la calidad del aire en tiempo real.

</div>

<div align="justify">

___

## Índice
- [Introducción](https://github.com/99danirmoya/Salero-MdN/tree/main?tab=readme-ov-file#introducci%C3%B3n)
- [Lista de componentes](https://github.com/99danirmoya/Salero-MdN/tree/main?tab=readme-ov-file#--lista-de-componentes-)
- [Esquemático](https://github.com/99danirmoya/Salero-MdN/tree/main?tab=readme-ov-file#esquem%C3%A1tico)
- [PCB personalizada](https://github.com/99danirmoya/Salero-MdN/tree/main?tab=readme-ov-file#pcb-personalizada)
- [Carcasa a medida](https://github.com/99danirmoya/Salero-MdN/tree/main?tab=readme-ov-file#carcasa-a-medida)
- [Modo de implementación](https://github.com/99danirmoya/Salero-MdN/tree/main?tab=readme-ov-file#-modo-de-implementaci%C3%B3n-)
- [Licencia](https://github.com/99danirmoya/Salero-MdN/tree/main?tab=readme-ov-file#-licencia-)
- [Contacto](https://github.com/99danirmoya/Salero-MdN/tree/main?tab=readme-ov-file#-contacto-)

___

## Introducción
Es un sensor que avisa con un mensaje de texto de la calidad del aire en un espacio interior.

El sensor dispone de una carcasa con tres zonas diferentes que se iluminan con distintos colores en función de las partículas de CO2 por millón de partículas.

Envía los valores tomados a una base de datos, y a través de una aplicación web, se puede monitorizar el estado de todas las estancias.

Además de la electrónica, el sensor consta de una carcasa de tres partes fácilmente montables: un difusor, un soporte de electrónica y un bastidor, al que puede acoplarse una peana que permite colocar el sensor sobre cualquier superficie plana.

___

## Modo de funcionamiento


</div>

> [!WARNING]
> El salero siempre está encendido dado su bajo consumo de batería. Si se retira la batería, no volverá a funcionar hasta que se le ponga la batería de nuevo, se conecte a un cargador, o ambas. Desde ese momento, se iniciará automáticamente

___

## <p align="justify">  Lista de componentes </p>

<div align="center">

| Componente | Unidades |
| ------------- | ------------- |
| Arduino NANO | 1 |
| [PCB](https://github.com/99danirmoya/Salero-MdN/blob/main/pcb) | 1 |
| Botón de inclinación | 1 |
| Botón | 1 |
| LED verde | 1 |
| LED amarillo | 1 |
| LED rojo | 1 |
| Resistencias 2K2 | 3 |
| Batería 18650 | 1 |
| Porta-batería | 1 |
| TP4056 | 1 |

</div>

<div align="justify">

___

## Esquemático

A continuación, se muestra el esquemático para consultar conexiones:

</div>

<div align="center">
  <img src="https://github.com/99danirmoya/Salero-MdN/blob/main/pics/Screenshot_19-6-2024_232017_easyeda.com.jpeg" width="1000"  style="margin: 10px;"/>
  
  <em>Esquemático del salero memorístico</em>
</div>
<br/>

___

## PCB personalizada

En la carpeta [`Salero-MdN/tree/main/pcb`](https://github.com/99danirmoya/Salero-MdN/tree/main/pcb) se encuentran los archivos Gerber para su fabricación:

<div align="center">
  <img src="https://github.com/99danirmoya/Salero-MdN/blob/main/pics/Screenshot_19-6-2024_183655_easyeda.com.jpeg" width="600"  style="margin: 10px;"/>

  <em>Vista posterior de la PCB</em>
</div>
<br/>

<div align="center">
  <img src="https://github.com/99danirmoya/Salero-MdN/blob/main/pics/Screenshot_19-6-2024_183749_easyeda.com.jpeg" width="600"  style="margin: 10px;"/>

  <em>Vista anterior de la PCB</em>
</div>
<br/>

___

### <p align="justify"> Flujograma </p>

```mermaid
graph TD;
  A[Configuración inicial: LEDs apagados] -->|1| B(Se echa sal)
  G -->|15| S(Sleep mode)
  S -->|16| T(INTERRUPCIÓN)
  T -->|17| B
  B -->|2| H(Se enciende el primer LED)
  H -->|3| I(¿Se echó sal de nuevo en la última hora?)
  I -->|4| J(Sí)
  I -->|5| K(No)
  K -->|12| G(Se apagan los LEDs)
  J -->|6| L(Se añade el segundo LED)
  L -->|7| M(¿Se echó sal de nuevo en la última hora?)
  M -->|8| N(Sí)
  M -->|10| O(No)
  N -->|9| P(Se añade el tercer LED)
  O -->|13| G
  P -->|11| R(Pasa 1 hora)
  R -->|14| G
  
```

___

<div align="justify">

## Carcasa a medida

En la carpeta [`Salero-MdN/tree/main/housing`](https://github.com/99danirmoya/Salero-MdN/tree/main/housing) se incluyen los tres archivos ".stl" que se muestran a continuación:

</div>

<div align="center">
  <img src="https://github.com/99danirmoya/Salero-MdN/blob/main/pics/Screenshot%202024-03-11%20191934.png" width="600"  style="margin: 10px;"/>

  <em>Dosificador</em>
</div>
<br/>
  
<div align="center">
  <img src="https://github.com/99danirmoya/Salero-MdN/blob/main/pics/Screenshot%202024-03-11%20191914.png" width="600"  style="margin: 10px;"/>

  <em>Recipiente de sal</em>
</div>
<br/>
  
<div align="center">
  <img src="https://github.com/99danirmoya/Salero-MdN/blob/main/pics/Screenshot%202024-03-11%20191955.png" width="600"  style="margin: 10px;"/>

  <em>Contenedor de electrónica</em>
</div>
<br/>

> [!WARNING]
> Los modelos de las imágenes son experimentales y aún están siendo trabajados

___

## <p align="justify"> Modo de implementación </p>

<div align="justify">

Toda la explicación del código de Arduino viene dada en el mismo, [`99danirmoya/Salero-MdN/tree/main/src/main.cpp`](https://github.com/99danirmoya/Salero-MdN/blob/main/src/main.cpp), en formato de comentarios al lado de cada línea

</div>

___

## <p align="justify"> Licencia </p>

<div align="justify">

Este proyecto está licenciado bajo la [GPL-3.0 license](https://github.com/99danirmoya/turtle-bot-neo?tab=GPL-3.0-1-ov-file).

</div>

___

## <p align="justify"> Contacto </p>

> [!IMPORTANT]
> Responderemos amablemente a dudas y leeremos sugerencias: [![Gmail Badge](https://img.shields.io/badge/-Gmail-c14438?style=for-the-badge&logo=Gmail&logoColor=white&link=mailto:medialablpwan@gmail.com)](mailto:medialablpwan@gmail.com)
> 
> Más información sobre nuestras actividades: [![Linkedin Badge](https://img.shields.io/badge/-LinkedIn-blue?style=for-the-badge&logo=Linkedin&logoColor=white&link=https://www.linkedin.com/groups/9298597/)](https://www.linkedin.com/groups/9298597/)

<div align="center">
  <img src="https://github.com/99danirmoya/Salero-MdN/blob/main/pics/-5976428790768845376_121.jpg" width="600">
</div>
<br/>

_<p align="justify"> Autores: Simón, Andrea, César, Aida, Juan, Iván y Carlos :shipit: </p>_

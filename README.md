# AWS-IoTSensorDataPredict

En este proyecto se va a realizar una estación meteorológica de bajo costo que permita leer los contaminantes del aire y la predicción de los contaminantes a lo largo del tiempo.
Para lograr esto, hemos usado un Arduino Mega, el cual nos va a servir para obtener la información de los sensores, los sensores que se van a utilizar.
son los sensores MQ, entre ellos se encuentran MQ2 (Humo), MQ5 (GLP), MQ7 (CO), MQ135 (CO2) y DHT22 (temperatura y humedad), donde cada uno tiene una lectura en específico, estas lecturas son
la cantidad de humo, GLP, Monóxido de Carbono (CO), Dióxido de Carbono (CO2), la temperatuna y humedad.

El otro paso que se quiere obtener es que la información obtenida por los sensores, sean enviados a la nube, en este caso, vamos a usar los servicios de Amazon (Amazon Web Server), donde se va a mostrar la data en tiempo real y guardarse la información. Esta información guardada luego va a ser usada para hacer predicciones, en este caso usamos Power BI para hacer la predicción de tipo ARIMA.

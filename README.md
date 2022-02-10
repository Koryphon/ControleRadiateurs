# ControleRadiateurs

Client MQTT écrit en C++17 et destiné à contrôler un ensemble de radiateurs pilotés par [FirmwareRadiateur](https://github.com/Koryphon/FirmwareRadiateur) implanté sur des cartes dédiées.

## Bibliothèques utilisées

Les bibliothèques utilisées sont les suivantes :

- ```libmosquitto```. Installé avec mosquitto, le broker MQTT. À installer avec Homebrew : ```brew install mosquitto```.
- ```libmosquittopp```. Wrapper C++ de la libmosquitto. Également installée avec mosquitto.
- ```nlohmann/json```. À installer via Homebrew : ```brew install nlohmann-json```.

## Compilation

```sh
c++ -o ctrlheaters -std=c++17 -Wno-psabi *.cpp -lmosquittopp -lpthread
```

## Fonctionnement

La politique à appliquer pour chaque radiateur est décrite dans le fichier ```config.json```. À la racine, on trouve les clés suivantes :

- ```"location"```. La valeur est de type chaine de caractères et identifie le lieu de déploiement. Il sera affiché à titre informatif au démarrage de l'application.
- ```"profiles"```. La valeur est une liste de profils de chauffe sur 24h. Un profil de chauffe peut être une liste de couples ```"hh:mm"```, ```<temperature>``` où ```hh``` spécifie l'heure à laquelle la température est appliquée et ```mm``` la minute. ```<temperature>``` est la température en flottant et en °C. Un profil peut également être une référence à une autre profil (alias).
- ```"heaters"```. La valeur est une liste de radiateurs avec une clé égale à leur topic MQTT. [FirmwareRadiateur](https://github.com/Koryphon/FirmwareRadiateur) nomme les radiateurs ```heaterX``` ou ```X``` est un nombre de 0 à 63. La valeur de chaque radiateur peut être la chaine ```"off"```, et dans ce cas le radiateur n'est pas pris en compte, ou bien un objet contenant (pour l'instant) un seul champ de clé ```"profile"``` et de valeur référençant un des profils présents dans la liste ```"profiles"```.

### Exemples de profils

Le morceau de json suivant :

```json
"profiles": {
    "salle": {
        "00:00": 16.0,
        "05:00": 20.0,
        "08:00": 18.0
    }
}
```

définit un profil de chauffe appelé ```"salle"``` et fixant une consigne de température de 16°C de minuit à 5h du matin, puis de 20°C de 5h du matin à 8h du matin puis de 18°C pour le reste de la journée et donc jusqu'à minuit.

Le morceau de json suivant :

```json
"profiles": {
    "salle": {
        "00:00": 16.0,
        "05:00": 20.0,
        "08:00": 18.0
    }
    "cuisine": "salle"
}
```

ajoute un profil de chauffe appelé ```"cuisine"``` qui est un alias du profil ```"salle"```.

### Exemples de définition de radiateurs

Le morceau de json suivant :

```json
"heaters": {
    "heater0": {
        "profile": "cuisine"
    },
    "heater1": {
        "profile": "salle"
    }
}
```

définit 2 radiateurs, ```"heater0"``` et ```"heater1"```. Le premier utilise le profil ```"cuisine"``` et le second le profil ```"salle"```.
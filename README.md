# Telemetry
Proiectul este alcatuit dintr-un daemon si o librarie care expune functionalitatea daemonului.

Daemonul se ocupa cu distributia de mesaje pe canale de distributie, intre mai multi participanti.

Canalele de distributie pot fi organizate ierarhic, in asa fel incat un mesaj trimis pe un canal 'parinte' va fi trimis si pe canalele 'copil'.

# Distribuire task-uri:
* **Pascaru Dan Alexandru** – Reprezentare canale
* **Petre Robert Cristian** – Comunicare librarie-daemon
* **Popescu Tiberiu** – Comunicare user-canale
* **Voicu Ioan Vladut** – Creare daemon

# Reprezentare canale

## Libraria dir.h
### Crearea si stergerea directoarelor
- **create_dir**: Permite crearea de directoare multiple(separate prin '/') si configurarea initiala a fisierelor de log(log.txt) in fiecare director nou creat.
- **remove_dir**: Permite stergerea directoarelor si a intregului continut din ele si din adancimea lor.

### Gestionarea fisierelor de log
- **open_log**: Deschide un fisier de log dintr-o ruta specificata si il returneaza.

### Parcurgerea directoarelor
- **get_dir**: Deschide si returneaza toate subdirectoarele dintr-un director sub forma unei liste simplu inlantuite sub forma de ruta absoluta.
- **Structura listei**: list->path, list->next si mereu se termina cu NULL.



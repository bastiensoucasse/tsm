# Effets

## Configuration

- Configurer la liste des plugins :

    ```sh
    source configure
    ```

## Compilation

- Compiler tous les plugins :

    ```sh
    make
    ```

- Compiler tous les plugins d'une personne :

    ```sh
    make <person>
    ```

- Compiler un plugin en particulier :

    ```sh
    make <plugin_person>
    ```

## Commandes

- Lister les plugins :

    ```sh
    listplugins
    ```

- Analyser un plugin (infos, entrée, sortie, paramètres de contrôle, ports, etc.) :

    ```sh
    analyseplugin <plugin>
    ```

- Appliquer un plugin :

    ```sh
    applypugin <input_wav> <output_wav> <plugin> <label> <controls>
    ```

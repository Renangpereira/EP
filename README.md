# SISTEMAS OPERACIONAIS

## LSB steganography PNG sistema de arquivos de álbum de fotos com chave de espaço do usuário criptografada para Linux.

#### Lucas Sampaio
#### Renan Gonçalves
#### Stéfani Pastre

Use o diretório de imagem PNG para criar, acessar e modificar o sistema de arquivos de texto criptografado LSB criptografado por chave no espaço do usuário. Somente depois que o sistema de arquivos for fechado com segurança por desinstalação ou Ctrl + C, o estado do sistema de arquivos pode ser preservado. O sistema de arquivos só pode usar imagens com o mesmo tamanho da imagem raiz fornecida.

O sistema de arquivos precisa de um nome, chave, imagem raiz e imagem de armazenamento. Para acessar o sistema de arquivos, você deve fornecer a chave, o nome do sistema de arquivos e a imagem raiz corretos. Ao ler e gravar imagens, todos os dados serão enviados ao XOR usando o hash SHA512 da chave. A imagem raiz armazena o nome do sistema de arquivos, espaço consumido e total, imagem e contagem de arquivos, hash de imagem e metadados
Arquivo. Todas as imagens adicionadas ao sistema de arquivos durante a formatação ou expansão estão no mesmo diretório da imagem raiz.

A formatação do sistema de arquivos apaga todos os bits menos significativos disponíveis na imagem fornecida. Da mesma forma, a exclusão de um arquivo apaga seus dados e, se houver uma lacuna, ele altera o sistema de arquivos. Todos os arquivos no sistema de arquivos têm 644 permissões e não podem ser editados, mas podem ser lidos, renomeados, excluídos e copiados.


### Como executar o programa:

#### Debian
```sh
$ apt-get install build-essential pkg-config libfuse-dev libpng-dev libssl-dev
```

#### CentOS
```sh
$ yum group install "Development Tools"
$ yum install fuse fuse-devel libpng libpng-devel openssl openssl-devel
```

#### Compilando:
```sh
$ make all
$ sudo make install
```



### Comandos e Descrição

| Command	| Description	|
|---------------|---------------|
| -format	| Crie um novo sistema de arquivos usando uma imagem válida no diretório de imagem raiz fornecido e monte-o.|
| -expand	| "Espanda" e monte o sistema de arquivos existente. Verifique se há outras imagens PNG válidas que ainda não estão contidas no sistema de arquivos no diretório que contém a imagem raiz.|
| -mount	| monta um sistema de arquivos existente|
| -debug	| habilita a saída de depuração bit a bit. O bloqueio de E / S desta saída fará com que o sistema de arquivos opere muito lentamente.|
| -help		| ajuda e saída do programa|

#### Exemplo de como executar:

```sh
$ albumfs -format images/vacation/image.png
```
```sh
$ albumfs -debug -mount images/vacation/image.png
```

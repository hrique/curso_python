/* Modelo Fisico: */

CREATE TABLE usuario (
    id_usuario INT PRIMARY KEY AUTO_INCREMENT,
    nome_usuario VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    senha VARCHAR(255) NOT NULL,
    data_criacao TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    fk_nivel_id_nivel INT NOT NULL
);

CREATE TABLE vinho (
    id_vinho INT PRIMARY KEY AUTO_INCREMENT,
    nome_vinho VARCHAR(150) NOT NULL,
    safra INT,
    preco DECIMAL(10, 2) NOT NULL,
    quantidade INT NOT NULL DEFAULT 0,
    fk_tipo_id_tipo INT NOT NULL,
    fk_paisorigem_id_paisorigem INT NOT NULL
);

CREATE TABLE tipo (
    id_tipo INT PRIMARY KEY AUTO_INCREMENT,
    nome_tipo VARCHAR(50) NOT NULL UNIQUE
);

CREATE TABLE uva (
    id_uva INT PRIMARY KEY AUTO_INCREMENT,
    nome_uva VARCHAR(100) NOT NULL UNIQUE
);

CREATE TABLE paisorigem (
    id_paisorigem INT PRIMARY KEY AUTO_INCREMENT,
    nome_pais VARCHAR(100) NOT NULL UNIQUE
);

CREATE TABLE nivel (
    id_nivel INT PRIMARY KEY AUTO_INCREMENT,
    nome_nivel VARCHAR(50) NOT NULL UNIQUE,
    descricao VARCHAR(255)
);

CREATE TABLE movimentacao (
    id_movimentacao INT PRIMARY KEY AUTO_INCREMENT,
    data_movimentacao TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    tipo_movimentacao ENUM('ENTRADA', 'SAIDA', 'AJUSTE_POSITIVO', 'AJUSTE_NEGATIVO') NOT NULL,
    quantidade_movimentada INT NOT NULL,
    fk_vinho_id_vinho INT NOT NULL,
    fk_usuario_id_usuario INT NOT NULL
);

CREATE TABLE composto_por (
    fk_uva_id_uva INT NOT NULL,
    fk_vinho_id_vinho INT NOT NULL,
    PRIMARY KEY (fk_uva_id_uva, fk_vinho_id_vinho)
);
 
ALTER TABLE usuario ADD CONSTRAINT FK_usuario_nivel
    FOREIGN KEY (fk_nivel_id_nivel)
    REFERENCES nivel (id_nivel)
    ON DELETE RESTRICT;

ALTER TABLE vinho ADD CONSTRAINT chk_preco_positivo
    CHECK (preco >= 0);

ALTER TABLE vinho ADD CONSTRAINT chk_quantidade_nao_negativa
    CHECK (quantidade >= 0);

ALTER TABLE vinho ADD CONSTRAINT FK_vinho_tipo
    FOREIGN KEY (fk_tipo_id_tipo)
    REFERENCES tipo (id_tipo)
    ON DELETE RESTRICT;
 
ALTER TABLE vinho ADD CONSTRAINT FK_vinho_pais
    FOREIGN KEY (fk_paisorigem_id_paisorigem)
    REFERENCES paisorigem (id_paisorigem)
    ON DELETE RESTRICT;

ALTER TABLE movimentacao ADD CONSTRAINT chk_quantidade_movimentada_positiva 
    CHECK (quantidade_movimentada > 0);

ALTER TABLE movimentacao ADD CONSTRAINT FK_movimentacao_vinho
    FOREIGN KEY (fk_vinho_id_vinho)
    REFERENCES vinho (id_vinho)
    ON DELETE RESTRICT;
 
ALTER TABLE movimentacao ADD CONSTRAINT FK_movimentacao_usuario
    FOREIGN KEY (fk_usuario_id_usuario)
    REFERENCES usuario (id_usuario)
    ON DELETE RESTRICT;
 
ALTER TABLE composto_por ADD CONSTRAINT FK_composto_uva
    FOREIGN KEY (fk_uva_id_uva)
    REFERENCES uva (id_uva)
    ON DELETE RESTRICT;
 
ALTER TABLE composto_por ADD CONSTRAINT FK_composto_vinho
    FOREIGN KEY (fk_vinho_id_vinho)
    REFERENCES vinho (id_vinho)
    ON DELETE CASCADE;
/* Modelo Fisico: */

CREATE TABLE nivel (
    id_nivel INT PRIMARY KEY AUTO_INCREMENT,
    nome_nivel VARCHAR(50) NOT NULL UNIQUE,
    descricao VARCHAR(255)
);

CREATE TABLE usuario (
    id_usuario INT PRIMARY KEY AUTO_INCREMENT,
    nome_usuario VARCHAR(100) NOT NULL,
    email VARCHAR(100) NOT NULL UNIQUE,
    senha CHAR(64) NOT NULL,
    data_criacao TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    fk_nivel_id_nivel INT NOT NULL,
    ativo TINYINT(1) NOT NULL DEFAULT 1,

    -- CONSTRAINT DE FK
    CONSTRAINT FK_usuario_nivel
        FOREIGN KEY(fk_nivel_id_nivel) REFERENCES nivel(id_nivel) ON DELETE RESTRICT
);

CREATE TABLE vinho (
    id_vinho INT PRIMARY KEY AUTO_INCREMENT,
    nome_vinho VARCHAR(150) NOT NULL,
    safra INT,
    preco DECIMAL(10, 2) NOT NULL CHECK (preco >= 0),
    quantidade INT NOT NULL DEFAULT 0 CHECK (quantidade >= 0),
    fk_tipo_id_tipo INT NOT NULL REFERENCES tipo(id_tipo) ON DELETE RESTRICT,
    fk_paisorigem_id_paisorigem INT NOT NULL REFERENCES paisorigem(id_paisorigem) ON DELETE RESTRICT,
    ativo TINYINT(1) NOT NULL DEFAULT 1
);

CREATE TABLE tipo (
    id_tipo INT PRIMARY KEY AUTO_INCREMENT,
    nome_tipo VARCHAR(50) NOT NULL UNIQUE,
    ativo TINYINT(1) NOT NULL DEFAULT 1
);

CREATE TABLE uva (
    id_uva INT PRIMARY KEY AUTO_INCREMENT,
    nome_uva VARCHAR(100) NOT NULL UNIQUE,
    ativo TINYINT(1) NOT NULL DEFAULT 1
);

CREATE TABLE paisorigem (
    id_paisorigem INT PRIMARY KEY AUTO_INCREMENT,
    nome_pais VARCHAR(100) NOT NULL UNIQUE,
    ativo TINYINT(1) NOT NULL DEFAULT 1
);

CREATE TABLE movimentacao (
    id_movimentacao INT PRIMARY KEY AUTO_INCREMENT,
    data_movimentacao TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    tipo_movimentacao ENUM('ENTRADA', 'SAIDA', 'AJUSTE_POSITIVO', 'AJUSTE_NEGATIVO') NOT NULL,
    quantidade_movimentada INT NOT NULL CHECK (quantidade_movimentada > 0),
    fk_vinho_id_vinho INT NOT NULL REFERENCES vinho(id_vinho) ON DELETE RESTRICT,
    fk_usuario_id_usuario INT NOT NULL REFERENCES usuario(id_usuario) ON DELETE RESTRICT
);

CREATE TABLE composto_por (
    fk_uva_id_uva INT NOT NULL REFERENCES uva(id_uva) ON DELETE RESTRICT,
    fk_vinho_id_vinho INT NOT NULL REFERENCES vinho(id_vinho) ON DELETE RESTRICT,
    PRIMARY KEY (fk_uva_id_uva, fk_vinho_id_vinho)
);

CREATE INDEX idx_usuario_nivel ON usuario (fk_nivel_id_nivel);
CREATE INDEX idx_vinho_tipo ON vinho (fk_tipo_id_tipo);
CREATE INDEX idx_vinho_pais ON vinho (fk_paisorigem_id_paisorigem);
CREATE INDEX idx_mov_vinho ON movimentacao (fk_vinho_id_vinho);
CREATE INDEX idx_mov_usuario ON movimentacao (fk_usuario_id_usuario);
CREATE INDEX idx_comp_vinho ON composto_por (fk_vinho_id_vinho);

INSERT INTO nivel (nome_nivel, descricao) 
	VALUES ('Admin', 'Administrador do sistema');
    
INSERT INTO usuario (nome_usuario, email, senha, fk_nivel_id_nivel, ativo) 
	VALUES ('Admin', 'admin@vintagem.com.br', '9287cc6c53b94a0527d41c16678a93d2e95361287bef29e66ee7970698d34edf', 1, 1);
    
CREATE TABLE log (
    id_log INT PRIMARY KEY AUTO_INCREMENT,
    data_hora TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    acao VARCHAR(500) NOT NULL,
    fk_usuario_id_usuario INT,
    FOREIGN KEY (fk_usuario_id_usuario) REFERENCES usuario(id_usuario)
);
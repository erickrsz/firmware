-- ============================================================
-- Calculadora Agrícola IoT — Schema do banco de dados
-- Baseado no dicionário de dados da seção 3.4 do TCC
-- Normalizado até a 3FN, conforme descrito na metodologia.
-- ============================================================

CREATE TABLE IF NOT EXISTS regiao (
    id_regiao    SERIAL PRIMARY KEY,
    nome_regiao  VARCHAR(50) NOT NULL,
    uf           CHAR(2) NOT NULL
);

CREATE TABLE IF NOT EXISTS tipo_solo (
    id_solo         SERIAL PRIMARY KEY,
    descricao_solo  VARCHAR(50) NOT NULL
);

CREATE TABLE IF NOT EXISTS cultura (
    id_cultura           SERIAL PRIMARY KEY,
    nome_cultura         VARCHAR(50) NOT NULL,
    ciclo_estimado_dias  INT NOT NULL
);

CREATE TABLE IF NOT EXISTS configuracao_safra (
    id_config     SERIAL PRIMARY KEY,
    id_regiao     INT NOT NULL REFERENCES regiao(id_regiao),
    id_solo       INT NOT NULL REFERENCES tipo_solo(id_solo),
    id_cultura    INT NOT NULL REFERENCES cultura(id_cultura),
    data_plantio  DATE NOT NULL
);

CREATE TABLE IF NOT EXISTS sensor_iot (
    id_sensor                SERIAL PRIMARY KEY,
    modelo                   VARCHAR(50) NOT NULL,
    fabricante               VARCHAR(50) NOT NULL,
    protocolo_comunicacao    VARCHAR(20) NOT NULL,
    intervalo_amostragem_min INT NOT NULL,
    status_sensor            VARCHAR(20) NOT NULL
);

CREATE TABLE IF NOT EXISTS leitura_iot (
    id_leitura   SERIAL PRIMARY KEY,
    id_config    INT NOT NULL REFERENCES configuracao_safra(id_config),
    id_sensor    INT NOT NULL REFERENCES sensor_iot(id_sensor),
    data_hora    TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    umidade_ar   FLOAT NOT NULL CHECK (umidade_ar BETWEEN 0 AND 100),
    temperatura  FLOAT NOT NULL CHECK (temperatura BETWEEN -20 AND 60)
);

-- Índices para as consultas mais comuns (por período e por sensor),
-- conforme mencionado na justificativa de modelagem física do TCC.
CREATE INDEX IF NOT EXISTS idx_leitura_data_hora ON leitura_iot (data_hora);
CREATE INDEX IF NOT EXISTS idx_leitura_config ON leitura_iot (id_config);
CREATE INDEX IF NOT EXISTS idx_leitura_sensor ON leitura_iot (id_sensor);

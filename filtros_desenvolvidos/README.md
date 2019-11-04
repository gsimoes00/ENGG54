# Processador multiefeitos para áudio

## Introdução:
  Os módulos embarcados DSPs podem ser utilizados no tratamento de áudio seja de voz ou na alteração de sinais para gerar efeitos não inerentes do Sinal. Tal trabalho tem como objetivo reproduzir na arquitetura TMS320C55x efeitos utilizados em guitarras elétricas.  O principal objeto de estudo nesse processo de desenvolvimento se fundamenta no processamento digital de audio e à utilização de componentes presentes no kit de desenvolvimento que ajudam neste processo. 
 
  O estudo levou em consideração a implementação inicial em matlab ou python dos modelos dos efeitos como filtros digitais aplicados à amostras de áudio no formato “.wav” com a finalidade de se verificar a funcionalidade dos mesmos. Tais amostras geralmente possuem taxa de amostragem de 44 kHz e com resolução de 16 bits.
  
Classificação de Efeitos de Áudio:
* Os efeitos aplicados em sinais de áudio podem ser classificados como:
* Filtros Básicos: Passa Baixas, Passa Altas, Equalizadores
* Filtros de tempo Variável: Wah-Wah, Phaser.
* Atrasadores: Vibrato, Flanger, Chorus, Echo
* Moduladores: modulador Ring, Tremolo, Vibrato
* Não-lineares: Compressão, Limitadores, Distorções, Excitações
* Efeitos Espaciais: Panning, Reverb, Surround Sound


## Processador Não Linear
  O eco residual pode ser reduzido ainda mais usando um Processador Não Linear (NLP) realizando um Center Clipper. O barulho do conforto é inserido para minimizar os efeitos adversos do NLP
  
  ### Center Clipper
  Não lineariedades no caminho do eco, barulho nos circuitos, e a fala próxima não correlacionada limitam a quantidade de cancelamentos para um cancelador de eco adaptável típico. O NLP mostrada na Figura 1 remove os últimos vestígios do eco restante. O NLP mais amplamente utilizada é o Center Clipper com entrada e saída características ilustradas na Figura 2. Essa operação não linear pode ser expressada como, Figura 3

                                     |d(n)| > 1/2 max{|x(n)|, ..., |x(n - L + 1)|}           Figura 1
                                     
                                     Px(n) = (1 - æ) * Px(n - 1) + æ|x(n)|                   Figura 2
                                     
                                     y(n) = {0, |x(n)| <= ß ; x(n),  |x(n)| > ß              Figura 3
                                     
  Onde ß é o limite de corte, Esse Center Clipper elimina completamente os sinais abaixo do limite de corte de ß, mas deixa os sinais maiores que o limiar de corte não afetado. Um grande valor de ß suprime todos os ecos residuais mas também deteriora a qualidade do próximo discurso. Geralmente o limiar é escolhido para ser igual ou exceder a amplitude do pico de retorno do eco.
  
  ### Barulho de Conforto (Comfort Noise)
  O NLP elimina completamente o eco residual e o ruído do circuito, tornando a conecção não real. Por exemplo, se o assinante local parar de falar, o nível de barulho irá cair para zero repentinamente pois foi cortado pelo NLP. Se a diferença é significativa, o assinante pode pensar que a ligação foi desconectada. Portanto, a supressão completa de sinal usando NLP tem um efeito indesejado. Este problema pode ser resolvido injetando-se baixo nível de ruído quando o eco residual é suprimido.

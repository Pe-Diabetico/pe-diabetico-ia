/*
 * model.h — Random Forest embarcado para ESP32
 * ==============================================
 * Gerado por: exportar_modelo_esp32.py
 * Configuração lida de: config.yaml
 * NÃO edite manualmente — re-execute o script para atualizar.
 *
 * Modelo:
 *   n_estimators   : 20
 *   max_depth      : 10
 *   depth real     : 9
 *   n_features     : 11
 *   n_nos_total    : 688
 *   RAM estimada   : 13.4 KB
 *
 * Features — array features[] no firmware DEVE seguir esta ordem:
 *   [ 0] idade
 *   [ 1] imc
 *   [ 2] anos_diabetes
 *   [ 3] hba1c
 *   [ 4] has
 *   [ 5] tabagismo
 *   [ 6] kpa_calcaneo
 *   [ 7] kpa_meta1
 *   [ 8] kpa_meta5
 *   [ 9] temp
 *   [10] umid
 *
 * Classes: 0 = BAIXO RISCO | 1 = ALTO RISCO
 */

#pragma once
#include <cstdarg>
namespace Eloquent {
    namespace ML {
        namespace Port {
            class RandomForest {
                public:
                    /**
                    * Predict class for features vector
                    */
                    int predict(float *x) {
                        uint8_t votes[2] = { 0 };
                        // tree #1
                        if (x[9] <= 32.54999923706055) {
                            if (x[7] <= 149.6999969482422) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[6] <= 170.25) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            if (x[3] <= 6.950000047683716) {
                                if (x[6] <= 162.45000457763672) {
                                    if (x[7] <= 134.70000457763672) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[6] <= 194.75) {
                                        if (x[6] <= 181.25) {
                                            if (x[3] <= 6.8500001430511475) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            if (x[7] <= 162.25) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                votes[0] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        if (x[7] <= 170.3499984741211) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[6] <= 193.0) {
                                    if (x[3] <= 7.950000047683716) {
                                        if (x[6] <= 190.79999542236328) {
                                            if (x[7] <= 160.95000457763672) {
                                                if (x[9] <= 34.14999961853027) {
                                                    if (x[6] <= 164.64999389648438) {
                                                        if (x[6] <= 161.75) {
                                                            votes[1] += 1;
                                                        }

                                                        else {
                                                            votes[0] += 1;
                                                        }
                                                    }

                                                    else {
                                                        votes[1] += 1;
                                                    }
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        // tree #2
                        if (x[6] <= 163.0) {
                            if (x[3] <= 6.950000047683716) {
                                if (x[10] <= 60.80000114440918) {
                                    if (x[2] <= 11.5) {
                                        if (x[6] <= 76.8499984741211) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[3] <= 6.450000047683716) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[2] <= 6.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[0] <= 58.5) {
                                        if (x[2] <= 8.5) {
                                            if (x[6] <= 194.35000610351562) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            if (x[10] <= 51.54999923706055) {
                                                if (x[3] <= 6.950000047683716) {
                                                    votes[1] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }

                                            else {
                                                if (x[9] <= 32.85000038146973) {
                                                    votes[1] += 1;
                                                }

                                                else {
                                                    if (x[10] <= 55.89999961853027) {
                                                        votes[1] += 1;
                                                    }

                                                    else {
                                                        votes[1] += 1;
                                                    }
                                                }
                                            }
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        // tree #3
                        if (x[9] <= 32.54999923706055) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[6] <= 194.6999969482422) {
                                if (x[1] <= 29.600000381469727) {
                                    if (x[6] <= 188.60000610351562) {
                                        if (x[2] <= 8.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            if (x[1] <= 29.34999942779541) {
                                                if (x[0] <= 55.0) {
                                                    votes[1] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }

                                            else {
                                                if (x[0] <= 65.5) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }
                                        }
                                    }

                                    else {
                                        if (x[2] <= 7.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[3] <= 6.900000095367432) {
                                        if (x[0] <= 67.0) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[3] <= 6.8500001430511475) {
                                    if (x[10] <= 49.39999961853027) {
                                        if (x[10] <= 47.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        // tree #4
                        if (x[2] <= 8.5) {
                            if (x[1] <= 26.449999809265137) {
                                if (x[8] <= 126.8499984741211) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[1] <= 29.199999809265137) {
                                    if (x[1] <= 28.949999809265137) {
                                        if (x[9] <= 33.39999961853027) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[9] <= 32.80000114440918) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[1] <= 29.649999618530273) {
                                if (x[1] <= 28.75) {
                                    if (x[0] <= 58.5) {
                                        if (x[3] <= 6.8500001430511475) {
                                            if (x[9] <= 33.5) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    if (x[8] <= 139.3499984741211) {
                                        if (x[8] <= 122.45000076293945) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[6] <= 144.1500015258789) {
                                    votes[1] += 1;
                                }

                                else {
                                    if (x[3] <= 6.900000095367432) {
                                        if (x[9] <= 32.89999961853027) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        // tree #5
                        if (x[7] <= 143.29999542236328) {
                            if (x[9] <= 32.54999923706055) {
                                if (x[9] <= 32.45000076293945) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[6] <= 136.75) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[2] <= 8.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[7] <= 141.0) {
                                        if (x[7] <= 130.1500015258789) {
                                            if (x[3] <= 7.1000001430511475) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[3] <= 6.400000095367432) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[6] <= 194.5500030517578) {
                                    if (x[8] <= 113.44999694824219) {
                                        if (x[9] <= 32.95000076293945) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            if (x[0] <= 62.5) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        if (x[1] <= 29.149999618530273) {
                                            if (x[4] <= 0.5) {
                                                if (x[2] <= 9.5) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[2] <= 9.5) {
                                        if (x[7] <= 202.25) {
                                            if (x[2] <= 7.5) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            if (x[1] <= 31.199999809265137) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        // tree #6
                        if (x[2] <= 8.5) {
                            if (x[1] <= 26.449999809265137) {
                                if (x[2] <= 1.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[7] <= 146.0999984741211) {
                                    if (x[10] <= 47.30000114440918) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[7] <= 161.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[3] <= 7.450000047683716) {
                                if (x[9] <= 33.04999923706055) {
                                    if (x[2] <= 10.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[10] <= 65.85000228881836) {
                                            if (x[8] <= 130.10000228881836) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                if (x[6] <= 182.75) {
                                                    votes[1] += 1;
                                                }

                                                else {
                                                    if (x[6] <= 225.75) {
                                                        votes[1] += 1;
                                                    }

                                                    else {
                                                        votes[1] += 1;
                                                    }
                                                }
                                            }
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        // tree #7
                        if (x[2] <= 8.5) {
                            if (x[8] <= 131.79999542236328) {
                                if (x[3] <= 7.5) {
                                    if (x[9] <= 32.85000038146973) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[1] <= 28.449999809265137) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[9] <= 33.39999961853027) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        if (x[0] <= 59.0) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[10] <= 41.30000114440918) {
                                    if (x[3] <= 7.849999904632568) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    if (x[6] <= 207.1500015258789) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[3] <= 7.1499998569488525) {
                                if (x[9] <= 33.35000038146973) {
                                    if (x[6] <= 210.85000610351562) {
                                        if (x[9] <= 32.75) {
                                            if (x[1] <= 29.84999942779541) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            if (x[3] <= 6.8500001430511475) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                if (x[2] <= 14.5) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    votes[0] += 1;
                                                }
                                            }
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                if (x[9] <= 32.95000076293945) {
                                    if (x[8] <= 118.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        if (x[9] <= 32.75) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        // tree #8
                        if (x[6] <= 164.3499984741211) {
                            if (x[3] <= 7.0) {
                                if (x[1] <= 29.25) {
                                    if (x[2] <= 9.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[2] <= 8.5) {
                                    votes[1] += 1;
                                }

                                else {
                                    if (x[9] <= 33.04999923706055) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[6] <= 194.45000457763672) {
                                if (x[9] <= 32.19999980926514) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[0] <= 58.5) {
                                        if (x[1] <= 28.449999809265137) {
                                            if (x[10] <= 68.54999923706055) {
                                                if (x[1] <= 27.149999618530273) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    votes[0] += 1;
                                                }
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            if (x[9] <= 33.04999923706055) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        if (x[7] <= 182.8000030517578) {
                                            if (x[1] <= 26.25) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[9] <= 32.85000038146973) {
                                    if (x[6] <= 239.6999969482422) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        // tree #9
                        if (x[9] <= 32.45000076293945) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[8] <= 86.95000076293945) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[6] <= 194.75) {
                                    if (x[3] <= 7.1499998569488525) {
                                        if (x[1] <= 29.600000381469727) {
                                            if (x[2] <= 9.5) {
                                                if (x[10] <= 34.95000076293945) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    votes[0] += 1;
                                                }
                                            }

                                            else {
                                                if (x[6] <= 181.25) {
                                                    if (x[2] <= 11.5) {
                                                        votes[1] += 1;
                                                    }

                                                    else {
                                                        votes[0] += 1;
                                                    }
                                                }

                                                else {
                                                    votes[0] += 1;
                                                }
                                            }
                                        }

                                        else {
                                            if (x[7] <= 137.6999969482422) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        if (x[2] <= 7.5) {
                                            if (x[1] <= 28.350000381469727) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                if (x[6] <= 177.75) {
                                                    votes[1] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }
                                        }

                                        else {
                                            if (x[6] <= 189.6999969482422) {
                                                if (x[0] <= 67.0) {
                                                    votes[1] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        // tree #10
                        if (x[9] <= 32.54999923706055) {
                            if (x[7] <= 149.6999969482422) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[6] <= 167.5500030517578) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            if (x[2] <= 7.5) {
                                if (x[0] <= 55.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                if (x[9] <= 33.35000038146973) {
                                    if (x[10] <= 56.60000038146973) {
                                        if (x[6] <= 194.5) {
                                            if (x[1] <= 28.350000381469727) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                if (x[10] <= 40.75) {
                                                    votes[1] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        // tree #11
                        if (x[6] <= 176.75) {
                            if (x[9] <= 32.64999961853027) {
                                if (x[1] <= 25.84999942779541) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[9] <= 31.799999237060547) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[3] <= 7.3500001430511475) {
                                    if (x[10] <= 64.5) {
                                        if (x[5] <= 0.5) {
                                            if (x[6] <= 163.39999389648438) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                if (x[4] <= 0.5) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            if (x[7] <= 157.25) {
                                if (x[9] <= 33.85000038146973) {
                                    if (x[7] <= 151.25) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                if (x[1] <= 26.550000190734863) {
                                    if (x[6] <= 199.75) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        if (x[0] <= 52.5) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        // tree #12
                        if (x[3] <= 6.950000047683716) {
                            if (x[1] <= 26.0) {
                                if (x[8] <= 111.0) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[9] <= 31.950000762939453) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[9] <= 33.35000038146973) {
                                    if (x[7] <= 143.29999542236328) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[4] <= 0.5) {
                                            if (x[6] <= 172.25) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                if (x[1] <= 29.050000190734863) {
                                                    votes[1] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[8] <= 146.9499969482422) {
                                        if (x[8] <= 122.70000076293945) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[2] <= 9.5) {
                                if (x[8] <= 136.35000610351562) {
                                    if (x[9] <= 32.85000038146973) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[0] <= 59.0) {
                                            if (x[1] <= 28.5) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            if (x[3] <= 7.200000047683716) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                if (x[0] <= 58.5) {
                                    if (x[6] <= 195.5500030517578) {
                                        if (x[9] <= 33.35000038146973) {
                                            if (x[10] <= 58.35000038146973) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        // tree #13
                        if (x[1] <= 25.65000057220459) {
                            if (x[0] <= 59.5) {
                                if (x[6] <= 180.95000457763672) {
                                    if (x[8] <= 49.10000038146973) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[2] <= 6.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[6] <= 194.45000457763672) {
                                    if (x[9] <= 33.35000038146973) {
                                        if (x[2] <= 9.5) {
                                            if (x[1] <= 29.75) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            if (x[7] <= 162.25) {
                                                if (x[3] <= 6.8500001430511475) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }

                                            else {
                                                votes[0] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        if (x[0] <= 52.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[8] <= 147.5500030517578) {
                                        if (x[6] <= 239.6500015258789) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            if (x[8] <= 145.4000015258789) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[0] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        if (x[1] <= 26.15000057220459) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        // tree #14
                        if (x[6] <= 163.0) {
                            if (x[9] <= 32.64999961853027) {
                                if (x[10] <= 59.85000038146973) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[2] <= 5.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[3] <= 7.1000001430511475) {
                                    if (x[2] <= 12.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    if (x[7] <= 142.25) {
                                        if (x[8] <= 100.75) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[9] <= 32.19999980926514) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[6] <= 194.6500015258789) {
                                    if (x[6] <= 191.95000457763672) {
                                        if (x[7] <= 180.9000015258789) {
                                            if (x[10] <= 47.0) {
                                                if (x[8] <= 114.94999694824219) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    if (x[1] <= 29.049999237060547) {
                                                        votes[1] += 1;
                                                    }

                                                    else {
                                                        votes[1] += 1;
                                                    }
                                                }
                                            }

                                            else {
                                                if (x[2] <= 8.5) {
                                                    votes[1] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        // tree #15
                        if (x[3] <= 6.950000047683716) {
                            if (x[9] <= 32.54999923706055) {
                                if (x[3] <= 6.75) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[7] <= 133.5) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[7] <= 139.1500015258789) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[7] <= 159.0500030517578) {
                                        if (x[7] <= 152.04999542236328) {
                                            if (x[7] <= 141.8499984741211) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[8] <= 136.1500015258789) {
                                if (x[1] <= 25.600000381469727) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[1] <= 29.550000190734863) {
                                        if (x[9] <= 32.89999961853027) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            if (x[8] <= 123.54999923706055) {
                                                if (x[10] <= 37.55000114440918) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    if (x[7] <= 152.5) {
                                                        votes[1] += 1;
                                                    }

                                                    else {
                                                        votes[1] += 1;
                                                    }
                                                }
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        if (x[3] <= 7.450000047683716) {
                                            if (x[7] <= 156.25) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            if (x[0] <= 55.0) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        // tree #16
                        if (x[9] <= 32.54999923706055) {
                            if (x[2] <= 9.5) {
                                if (x[0] <= 35.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[0] <= 58.5) {
                                if (x[7] <= 182.25) {
                                    if (x[7] <= 180.5999984741211) {
                                        if (x[7] <= 151.1500015258789) {
                                            if (x[2] <= 8.5) {
                                                if (x[6] <= 160.0) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    votes[0] += 1;
                                                }
                                            }

                                            else {
                                                if (x[9] <= 33.04999923706055) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }
                                        }

                                        else {
                                            if (x[9] <= 33.35000038146973) {
                                                if (x[6] <= 196.0) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[2] <= 9.5) {
                                        if (x[3] <= 6.950000047683716) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[9] <= 32.85000038146973) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[1] <= 29.600000381469727) {
                                    if (x[4] <= 0.5) {
                                        if (x[3] <= 7.5) {
                                            if (x[7] <= 185.25) {
                                                if (x[9] <= 33.25) {
                                                    votes[0] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    if (x[9] <= 33.35000038146973) {
                                        if (x[3] <= 6.900000095367432) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            if (x[9] <= 32.75) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        // tree #17
                        if (x[3] <= 6.950000047683716) {
                            if (x[7] <= 157.25) {
                                if (x[6] <= 159.0) {
                                    if (x[7] <= 139.70000457763672) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        if (x[6] <= 148.8499984741211) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }
                                }

                                else {
                                    if (x[0] <= 57.5) {
                                        if (x[10] <= 59.25) {
                                            if (x[8] <= 129.25) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                votes[0] += 1;
                                            }
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[10] <= 59.20000076293945) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[0] <= 52.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    if (x[8] <= 160.04999542236328) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        if (x[6] <= 210.5) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }
                            }
                        }

                        else {
                            if (x[1] <= 25.600000381469727) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[9] <= 33.35000038146973) {
                                    if (x[0] <= 58.5) {
                                        if (x[3] <= 7.5) {
                                            if (x[8] <= 132.64999771118164) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        if (x[3] <= 7.1499998569488525) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        // tree #18
                        if (x[9] <= 32.54999923706055) {
                            if (x[8] <= 124.8499984741211) {
                                if (x[4] <= 0.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        else {
                            if (x[0] <= 58.5) {
                                if (x[6] <= 163.0999984741211) {
                                    if (x[1] <= 28.949999809265137) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[4] <= 0.5) {
                                        if (x[6] <= 196.1999969482422) {
                                            if (x[2] <= 8.5) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                if (x[1] <= 31.25) {
                                                    if (x[9] <= 33.60000038146973) {
                                                        votes[0] += 1;
                                                    }

                                                    else {
                                                        votes[1] += 1;
                                                    }
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }
                                        }

                                        else {
                                            if (x[0] <= 57.5) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        if (x[9] <= 33.19999885559082) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            if (x[2] <= 9.5) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }
                                }
                            }

                            else {
                                if (x[9] <= 32.75) {
                                    if (x[4] <= 0.5) {
                                        votes[1] += 1;
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    if (x[3] <= 6.950000047683716) {
                                        if (x[1] <= 28.850000381469727) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            if (x[1] <= 30.899999618530273) {
                                                if (x[10] <= 46.14999961853027) {
                                                    votes[1] += 1;
                                                }

                                                else {
                                                    votes[1] += 1;
                                                }
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        // tree #19
                        if (x[3] <= 6.950000047683716) {
                            if (x[9] <= 32.85000038146973) {
                                if (x[0] <= 57.5) {
                                    if (x[10] <= 42.04999923706055) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    if (x[3] <= 6.450000047683716) {
                                        votes[0] += 1;
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }
                            }

                            else {
                                if (x[7] <= 134.5500030517578) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        else {
                            if (x[2] <= 9.5) {
                                if (x[8] <= 136.35000610351562) {
                                    if (x[4] <= 0.5) {
                                        if (x[5] <= 0.5) {
                                            if (x[8] <= 128.45000457763672) {
                                                votes[0] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            votes[1] += 1;
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        // tree #20
                        if (x[3] <= 6.950000047683716) {
                            if (x[1] <= 26.0) {
                                if (x[7] <= 154.5) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                if (x[8] <= 128.10000228881836) {
                                    if (x[0] <= 57.5) {
                                        if (x[3] <= 6.8500001430511475) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            votes[0] += 1;
                                        }
                                    }

                                    else {
                                        if (x[9] <= 33.35000038146973) {
                                            votes[0] += 1;
                                        }

                                        else {
                                            if (x[3] <= 6.8500001430511475) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }
                                }

                                else {
                                    if (x[0] <= 66.5) {
                                        if (x[6] <= 198.0999984741211) {
                                            votes[1] += 1;
                                        }

                                        else {
                                            if (x[9] <= 33.25) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        votes[1] += 1;
                                    }
                                }
                            }
                        }

                        else {
                            if (x[9] <= 32.64999961853027) {
                                if (x[8] <= 123.20000076293945) {
                                    votes[0] += 1;
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }

                            else {
                                if (x[6] <= 193.0) {
                                    if (x[6] <= 191.14999389648438) {
                                        if (x[2] <= 9.5) {
                                            if (x[8] <= 122.75) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }

                                        else {
                                            if (x[3] <= 7.3500001430511475) {
                                                votes[1] += 1;
                                            }

                                            else {
                                                votes[1] += 1;
                                            }
                                        }
                                    }

                                    else {
                                        votes[0] += 1;
                                    }
                                }

                                else {
                                    votes[1] += 1;
                                }
                            }
                        }

                        // return argmax of votes
                        uint8_t classIdx = 0;
                        float maxVotes = votes[0];

                        for (uint8_t i = 1; i < 2; i++) {
                            if (votes[i] > maxVotes) {
                                classIdx = i;
                                maxVotes = votes[i];
                            }
                        }

                        return classIdx;
                    }

                    /**
                    * Predict readable class name
                    */
                    const char* predictLabel(float *x) {
                        return idxToLabel(predict(x));
                    }

                    /**
                    * Convert class idx to readable name
                    */
                    const char* idxToLabel(uint8_t classIdx) {
                        switch (classIdx) {
                            case 0:
                            return "BAIXO";
                            case 1:
                            return "ALTO";
                            default:
                            return "Houston we have a problem";
                        }
                    }

                protected:
                };
            }
        }
    }
import csv


def collect_cli():
    paciente = {
        'id': input('ID do paciente: '),
        'idade': input('Idade: '),
        'sexo': input('Sexo (M/F): '),
        'pti': input('PTI: '),
    }
    with open('data/pacientes.csv', 'a', newline='', encoding='utf-8') as f:
        writer = csv.DictWriter(f, fieldnames=['id','idade','sexo','pti'])
        writer.writerow(paciente)
    print('Paciente salvo.')


if __name__ == '__main__':
    collect_cli()

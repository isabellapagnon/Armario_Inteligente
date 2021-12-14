#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "print.h"
#include "hd44780.h"

char senhaCadastrada[7];
char senhaRepetida[7];
char senhaAbrir[7];

// decodifica leitura do keypad
char scan_keypad()
{
	char keys[4][3] = {{'1', '2', '3'}, {'4', '5', '6'}, {'7', '8', '9'}, {'*', '0', '#'}};
	uint8_t i, j;
	int k = 0;

	for (j = 0; j < 3; j++)
	{
		if (j == 2)
			PORTD |= (1 << PD5);
		if (j == 1)
			PORTD |= (1 << PD6);
		if (j == 0)
			PORTD |= (1 << PD7);
		for (i = 0; i < 4; i++)
		{
			// Podemos fazer assim porque PD0 é 0, PD1 é 1, etc.
			if (PIND & (1 << i))
			{
				_delay_ms(5);
				// Esperamos a tecla ser solta, para evitar repetição de entrada
				while (PIND & (1 << i))
					;
				break;
			}
		}

		PORTD &= ~((1 << PD5) | (1 << PD6) | (1 << PD7));

		if (i < 4)
		{
			// salvandoSenha(i,j,k);
			// k++;
			return keys[i][j];
		}
	}

	return 0;
}

void printInConsole(char senha[], int count)
{
	switch (senha[count])
	{
	case '1':
		hd44780_puts("1");
		break;

	case '2':
		hd44780_puts("2");
		break;

	case '3':
		hd44780_puts("3");
		break;

	case '4':
		hd44780_puts("4");
		break;

	case '5':
		hd44780_puts("5");
		break;

	case '6':
		hd44780_puts("6");
		break;

	case '7':
		hd44780_puts("7");
		break;

	case '8':
		hd44780_puts("8");
		break;

	case '9':
		hd44780_puts("9");
		break;

	case '0':
		hd44780_puts("0");
		break;

	case 1:
		hd44780_puts("1");
		break;
	default:
		hd44780_puts("erro");
	}
}

void estadoLivre()
{
	// led verde ligado
	// "Livre para uso"

	hd44780_clear();
	hd44780_puts("Livre para uso");
	PORTB |= (1 << PB5);  // Liga o led verde
	PORTB &= ~(1 << PB7); // Desliga o led vemelhor
}

int verificar(uint16_t key, int count, char senha[])
{
	if (key == '*' && count < 6)
	{
		if (senha == senhaRepetida)
		{
			hd44780_clear();
			for (int i = 0; i < 6; i++)
			{
				printInConsole(senhaCadastrada, i);
			}
			hd44780_gotoxy(1, 1);
			for (int i = 0; i < count - 1; i++)
			{
				printInConsole(senha, i);
			}
		}

		if ((senha == senhaCadastrada || senha == senhaAbrir) && count < 6)
		{
			hd44780_clear();
			for (int i = 0; i < count - 1; i++)
			{
				printInConsole(senha, i);
			}
		}
		return -1;
	}
	if (key == '#' && count == 6)
	{
		return 0;
	}

	return 1;
}

void cadastrandoSenha(uint16_t key, char senha[])
{
	int count = 0;
	for (;;)
	{
		while (count < 7) // Senha de 6 digitos
		{
			key = scan_keypad();

			if (verificar(key, count, senha) == -1)
			{
				count = count - 1;
			}

			else if (verificar(key, count, senha) == 1)
			{
				senha[count] = key;

				if (key)
				{
					printInConsole(senha, count);
					count = count + 1;
				}
			}
			else
			{ //returns 0
				break;
			}
		}
		break;
	}
}

int comparandoSenhas(char senhaCadastrada[], char senhaRepetida[])
{
	for (int i = 0; i < 7; i++)
	{
		if (senhaCadastrada[i] != senhaRepetida[i])
		{
			return -1;
		}
	}
	return 1;
}

void comparandoSenhas2(char senhaCadastrada[], char senhaRepetida[])
{
	if (comparandoSenhas(senhaCadastrada, senhaRepetida) == -1)
	{
		hd44780_clear();
		hd44780_puts("Senhas diferentes");
		_delay_ms(2000);
		//estadoLivre();
		main();
	}
	else
	{
		hd44780_clear();
		hd44780_puts("Senha cadastrada");
		_delay_ms(2000);
		estadoOcupado();
	}
}

void estadoOcupado()
{
	hd44780_clear();
	hd44780_puts("Em uso");
	PORTB |= (1 << PB7);  // Liga o led vermelho
	PORTB &= ~(1 << PB5); // Desliga o led verde
}

void abrir(uint16_t key, char senha[])
{
	int count = 0;
	hd44780_clear();
	for (;;)
	{
		while (count < 7) // Senha de 6 digitos
		{
			key = scan_keypad();

			if (verificar(key, count, senha) == -1)
			{
				count = count - 1;
			}

			else if (verificar(key, count, senha) == 1)
			{
				senha[count] = key;

				if (key)
				{
					printInConsole(senha, count);
					count = count + 1;
				}
			}
			else
			{ //returns 0
				break;
			}
		}
		break;
	}
}

int comparar(char senhaCadastrada[], char senha[])
{
	for (int i = 0; i < 6; i++)
	{
		if (senhaCadastrada[i] != senha[i])
		{
			hd44780_clear();
			hd44780_puts("Senhas diferentes");
			_delay_ms(2000);
			return -1;
		}
	}

	return 1;
}

void abrirCofre(uint16_t key, char senha[])
{
	PORTB &= ~(1 << PB7);
	hd44780_clear();
	int count = 0;
	int tentativas = 0;
	while (tentativas < 3)
	{
		abrir(key, senha);

		if (comparar(senhaCadastrada, senha) == -1)
		{
			tentativas++;
		}

		else
		{
			hd44780_clear();
			//estadoLivre();
			main();
			break;
		}
	}

	if (tentativas == 3)
	{
		hd44780_clear();
		PORTB |= (1 << PB7);
		hd44780_puts("bloqueado");
		_delay_ms(30000);
		abrirCofre(key, senha);
	}
}

int main(void)
{
	uint16_t key;

	USART_Init();

	// configura PD0 ~ PD3 como entradas (com pull-down externo)
	DDRD &= ~((1 << PD0) | (1 << PD1) | (1 << PD2) | (1 << PD3));
	// configura PD5 ~ PD7 como saídas
	DDRD |= (1 << PD5) | (1 << PD6) | (1 << PD7);

	DDRB |= (1 << PB5); // Verde
	DDRB |= (1 << PB7); // Vermelho
	DDRB &= ~(1 << PB0);
	PORTB |= (1 << PB0);

	hd44780_init();

	estadoLivre();

	_delay_ms(5000);
	hd44780_clear();

	cadastrandoSenha(key, senhaCadastrada);
	hd44780_gotoxy(1, 1);
	cadastrandoSenha(key, senhaRepetida);

	comparandoSenhas2(senhaCadastrada, senhaRepetida);

	_delay_ms(5000);

	abrirCofre(key, senhaAbrir);

	return 0;
}

/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/28 10:15:20 by rguigneb          #+#    #+#             */
/*   Updated: 2025/04/08 16:41:48 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "mlx.h"
#include "mlx_int.h"
#include <math.h>
#include <stdio.h>

#define PI 3.1415926535

#define MAP_HEIGHT 5
#define MAP_WIDTH 5
#define PLAYER_SPEED 1
#define SIZE 16

typedef struct s_player
{
	float		px;
	float		py;
	float		rx;
	float		ry;
	float		ra;
}				t_player;

typedef struct s_data
{
	void		*mlx;
	void		*win;
	t_player	player;
	char		map[MAP_HEIGHT][MAP_WIDTH];
}				t_data;

void	init_map(t_data *data)
{
	char temp[MAP_HEIGHT][MAP_WIDTH] = {
		{1, 1, 1, 1, 1},
		{1, 0, 0, 0, 1},
		{1, 0, 0, 0, 1},
		{1, 0, 0, 1, 1},
		{1, 1, 1, 1, 1},
	};
	for (int i = 0; i < MAP_HEIGHT; i++)
	{
		for (int j = 0; j < MAP_WIDTH; j++)
		{
			data->map[i][j] = temp[i][j];
		}
	}
}

void	on_key_pressed(int key, t_data *data)
{
	if (key == 'a')
	{
		data->player.ra -= 0.1;
		if (data->player.ra < 0)
			data->player.ra += 2 * PI; // += ?
		data->player.rx = cos(data->player.ra) * 5;
		data->player.ry = sin(data->player.ra) * 5;
	}
	if (key == 'd')
	{
		data->player.ra += 0.1;
		if (data->player.ra > 2 * PI)
			data->player.ra -= 2 * PI; // += ?
		data->player.rx = cos(data->player.ra) * 5;
		data->player.ry = sin(data->player.ra) * 5;
	}
	if (key == 's')
	{
		data->player.px -= data->player.rx;
		data->player.py -= data->player.ry;
	}
	if (key == 'w')
	{
		data->player.px += data->player.rx;
		data->player.py += data->player.ry;
	}
	printf("key :%d\n", key);
}

void send_3d_rays(t_data *data)
{

	int r, mx, my, mp, dof; float rx, ry, ra, xo, yo;

	ra = data->player.ra;

	for (r = 0; r < 1; r++)
	{
		dof = 0;
		float aTan = -1 / tan(ra);
		if (ra > PI)
		{ // looking up
			ry = (((int)data->player.py>>6)<<6)-0.0001;
			rx = (data->player.py - ry) * aTan + data->player.px;
			yo = -64;
			xo =-yo * aTan;
		}
		if (ra < PI) {
			ry = (((int)data->player.py>>6)<<6)+ 64;
			rx = (data->player.py - ry) * aTan + data->player.px;
			yo = 64;
			xo =-yo * aTan;
		}
		if (ra == 0 || ra == PI)
		{
			rx = data->player.px;
			ry = data->player.py;
			dof = 8;
		}
		while (dof < 8)
		{
			mx = (int)(rx)>>6; my = (int)(ry)>>6; // divided by 64
			if (mx <= MAP_WIDTH && my <= MAP_HEIGHT && data->map[my][mx] == 1)
				break ; // dof = 8;
			else
			{
				rx += xo;
				ry += yo;
				dof += 1;
			}
		}

		float nTan = -1 / tan(ra);
		if (ra > PI)
		{ // looking up
			ry = (((int)data->player.py>>6)<<6)-0.0001;
			rx = (data->player.py - ry) * nTan + data->player.px;
			yo = -64;
			xo =-yo * nTan;
		}
		if (ra < PI) {
			ry = (((int)data->player.py>>6)<<6)+ 64;
			rx = (data->player.py - ry) * nTan + data->player.px;
			yo = 64;
			xo =-yo * nTan;
		}
		if (ra == 0 || ra == PI)
		{
			rx = data->player.px;
			ry = data->player.py;
			dof = 8;
		}
		while (dof < 8)
		{
			mx = (int)(rx)>>6; my = (int)(ry)>>6; // divided by 64
			if (mx <= MAP_WIDTH && my <= MAP_HEIGHT && data->map[my][mx] == 1)
				break ; // dof = 8;
			else
			{
				rx += xo;
				ry += yo;
				dof += 1;
			}
		}

		for (int l = 0; l < SIZE / 4; l++)
		{
			for (int k = 0; k < SIZE / 4; k++)
			{
				mlx_pixel_put(data->mlx, data->win, (((int)data->player.px
							+ rx)) + l, (((int)data->player.py
							+ ry)) + k, 0xDD0000);
			}
		}
	}

}

void	loop(t_data *data)
{
	// printf("pos : %f, %f\n", data->player.px, data->player.py);
	mlx_clear_window(data->mlx, data->win);
	usleep(10000);
	for (int i = 0; i < MAP_HEIGHT; i++)
	{
		for (int j = 0; j < MAP_WIDTH; j++)
		{
			for (int l = 0; l < SIZE - 2; l++)
			{
				for (int k = 0; k < SIZE - 2; k++)
				{
					if (data->map[i][j] == 1)
						mlx_pixel_put(data->mlx, data->win, (i * SIZE) + l, (j
								* SIZE) + k, 0xFFDD00);
					else
						mlx_pixel_put(data->mlx, data->win, (i * SIZE) + l, (j
								* SIZE) + k, 0x00DDDD);
				}
			}
		}
	}
	for (int l = 0; l < SIZE / 2; l++)
	{
		for (int k = 0; k < SIZE / 2; k++)
		{
			mlx_pixel_put(data->mlx, data->win, ((int)data->player.px * SIZE
					/ 2) + l, ((int)data->player.py * SIZE / 2) + k, 0xFF0000);
		}
	}
	send_3d_rays(data);
	// for (int l = 0; l < SIZE / 2; l++)
	// {
	// 	for (int k = 0; k < SIZE / 2; k++)
	// 	{
	// 		mlx_pixel_put(data->mlx, data->win, (((int)data->player.px
	// 					+ data->player.rx) * 10) + l, (((int)data->player.py
	// 					+ data->player.ry) * 10) + k, 0xFF0000);
	// 	}
	// }
}

int	destroy_close(t_data *data)
{
	mlx_loop_end(data->mlx);
	return (0);
}

int	main(int argc, char const *argv[])
{
	t_data	data;

	init_map(&data);
	data.player.px = 4;
	data.player.py = 4;
	data.player.ra = 0.1;
	data.player.rx = cos(data.player.ra) * 5;
	data.player.ry = sin(data.player.ra) * 5;
	data.mlx = mlx_init();
	if (!data.mlx)
		return (EXIT_FAILURE);
	data.win = mlx_new_window(data.mlx, 500, 500, "RayCaster in C");
	if (!data.win)
		return (mlx_destroy_display(data.mlx), free(data.mlx), EXIT_FAILURE);
	mlx_hook(data.win, DestroyNotify, 0, destroy_close, &data.mlx);
	mlx_key_hook(data.win, (int (*)(void *))on_key_pressed, &data);
	mlx_loop_hook(data.mlx, (int (*)(void *))loop, &data);
	mlx_loop(data.mlx);
	mlx_destroy_window(data.mlx, data.win);
	mlx_destroy_display(data.mlx);
	free(data.mlx);
	return (0);
}

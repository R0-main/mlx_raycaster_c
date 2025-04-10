/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/28 10:15:20 by rguigneb          #+#    #+#             */
/*   Updated: 2025/04/10 12:23:40 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "mlx.h"
#include "mlx_int.h"
#include <math.h>
#include <stdio.h>

#define PI 3.1415926535

#define MAP_HEIGHT 5
#define MAP_WIDTH 5
#define SIZE 64
#define FOV 60
#define X_SIZE = 4
#define RAYS_COUNT 

#define SCREEN_HEIGHT MAP_HEIGHT *SIZE
#define SCREEN_WIDTH MAP_WIDTH *SIZE

#define PLAYER_SPEED 1

typedef struct s_uvec_2
{
	unsigned int	x;
	unsigned int	y;
}					t_uvec2;

typedef struct s_fvec_2
{
	float			x;
	float			y;
}					t_fvec2;

typedef struct s_player
{
	t_fvec2			position;
	float			rotation_angle;
}					t_player;

typedef struct s_data
{
	void			*mlx;
	void			*win;
	t_img			*rendering_buffer;
	t_player		player;
	char			*map;
}					t_data;


float	normalize_angle(float angle)
{
	angle = fmodf(angle, (2.0f * PI));
	if (angle < 0)
		angle += (2.0f * PI);
	return (angle);
}

void	on_key_pressed(int key, t_data *data)
{
	float	adj;
	float	opo;

	printf("key :%d\n", key);
	if (key == 'w')
	{
		adj = cos(data->player.rotation_angle) * 3.5;
		opo = sin(data->player.rotation_angle) * 3.5;
		printf("adj : %d", adj);
		printf("opo : %d", opo);
		data->player.position.x += adj;
		data->player.position.y += opo;
	}
	if (key == 's')
	{
		adj = cos(data->player.rotation_angle) * 3.5;
		opo = sin(data->player.rotation_angle) * 3.5;
		printf("adj : %d", adj);
		printf("opo : %d", opo);
		data->player.position.x -= adj;
		data->player.position.y -= opo;
	}
	if (key == 'a')
	{
		data->player.rotation_angle -= 0.25;
	}
	if (key == 'd')
	{
		data->player.rotation_angle += 0.25;
	}
	data->player.rotation_angle = normalize_angle(data->player.rotation_angle);
}

// t_img *create_re
int	ft_max(int a, int b)
{
	if (a > b)
		return (a);
	return (b);
}

void	put_pixel_to_buffer(t_img *buffer, t_uvec2 pos, int color)
{
	((unsigned int *)buffer->data)[(pos.y * (buffer->size_line / 4))
		+ pos.x] = color;
}

void	draw_line(t_img *buffer, int color, t_uvec2 start, t_uvec2 end)
{
	int		distance_x;
	int		distance_y;
	float	step;
	float	stepX;
	float	stepY;
	int		i;
	t_uvec2	tmp;
	int		y;
	int		x;

	i = 0;
	if (end.y - start.y >= end.x - start.x)
	{
		tmp = start;
		start = end;
		end = tmp;
	}
	distance_x = end.x - start.x;
	distance_y = end.y - start.y;
	step = ft_max(abs(distance_x), abs(distance_y));
	if (step != 0)
	{
		stepX = distance_x / step;
		stepY = distance_y / step;
	}
	while (i < step + 1)
	{
		y = roundf(start.y + i * stepY);
		x = roundf((float)(start.x + i * stepX));
		put_pixel_to_buffer(buffer, (t_uvec2){x, y}, color);
		i++;
	}
}

void	draw_rect(t_img *buffer, int color, t_uvec2 start, t_uvec2 end)
{
	t_uvec2	tmp;

	tmp = start;
	while (tmp.x < end.x)
	{
		tmp.y = start.y;
		while (tmp.y < end.y)
		{
			put_pixel_to_buffer(buffer, tmp, color);
			tmp.y++;
		}
		tmp.x++;
	}
}

void	draw_map(t_img *buffer, char *map)
{
	t_uvec2	tmp;

	tmp = (t_uvec2){0, 0};
	while (tmp.x < MAP_WIDTH)
	{
		tmp.y = 0;
		while (tmp.y < MAP_HEIGHT)
		{
			if (map[tmp.y * MAP_HEIGHT + tmp.x] == 1)
				draw_rect(buffer, 0x00FFFF, (t_uvec2){tmp.x * SIZE, tmp.y
					* SIZE}, (t_uvec2){tmp.x * SIZE + SIZE - 2, tmp.y * SIZE
					+ SIZE - 2});
			else
				draw_rect(buffer, 0xFFFFFF, (t_uvec2){tmp.x * SIZE, tmp.y
					* SIZE}, (t_uvec2){tmp.x * SIZE + SIZE - 2, tmp.y * SIZE
					+ SIZE - 2});
			tmp.y++;
		}
		tmp.x++;
	}
}

void	draw_ray(t_img *buffer, t_player player, float angle, int len)
{
	draw_line(buffer, 0xFF0000, (t_uvec2){player.position.x, player.position.y},
		(t_uvec2){player.position.x + cos(angle) * len, player.position.y
		+ sin(angle) * len});
}

void	raycaster(t_img *buffer, t_player player, float angle, int len)
{
	draw_line(buffer, 0xFF0000, (t_uvec2){player.position.x, player.position.y},
		(t_uvec2){player.position.x + cos(angle) * len, player.position.y
		+ sin(angle) * len});
}

void	draw_player(t_img *buffer, t_player player)
{
	float	i;

	draw_rect(buffer, 0xFF0000, (t_uvec2){((int)player.position.x) - 5,
		((int)player.position.y) - 5}, (t_uvec2){((int)player.position.x) + 5,
		((int)player.position.y) + 5});
	draw_ray(buffer, player, player.rotation_angle, 50);
	i = player.rotation_angle - FOV / 2 ;
	while (i <= player.rotation_angle + FOV / 2)
	{
		draw_ray(buffer, player, i, 25);
		i += 0.2;
		// i++;
	}
	// draw_rect(buffer, 0x0DF0FF, player.position, (t_uvec2){player.position.x
	// +10, player.position.y + 10
	// });
}

void	loop(t_data *data)
{
	usleep(10000);
	mlx_destroy_image(data->mlx, data->rendering_buffer);
	data->rendering_buffer = mlx_new_image(data->mlx, SCREEN_WIDTH,
			SCREEN_HEIGHT);
	draw_line(data->rendering_buffer, 0xFF0000, (t_uvec2){50, 200}, (t_uvec2){0,
		0});
	draw_line(data->rendering_buffer, 0xDD00DD, (t_uvec2){0, 0}, (t_uvec2){50,
		90});
	draw_rect(data->rendering_buffer, 0x00DF0F, (t_uvec2){90, 90},
		(t_uvec2){120, 99});
	draw_map(data->rendering_buffer, data->map);
	draw_player(data->rendering_buffer, data->player);
	mlx_put_image_to_window(data->mlx, data->win, data->rendering_buffer, 0, 0);
}

int	destroy_close(t_data *data)
{
	mlx_loop_end(data->mlx);
	return (0);
}

int	main(int argc, char const *argv[])
{
	t_data	data;

	char map[MAP_HEIGHT * MAP_WIDTH] = {
		1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1,
			1,
	};
	bzero(&data, sizeof(t_data));
	data.player.position.x = SCREEN_WIDTH / 2;
	data.player.position.y = SCREEN_HEIGHT / 2;
	data.player.rotation_angle = 45 * (PI / 180);
	data.map = (char *)&map;
	data.mlx = mlx_init();
	if (!data.mlx)
		return (EXIT_FAILURE);
	data.win = mlx_new_window(data.mlx, SCREEN_WIDTH, SCREEN_HEIGHT,
			"RayCaster in C");
	if (!data.win)
		return (mlx_destroy_display(data.mlx), free(data.mlx), EXIT_FAILURE);
	data.rendering_buffer = mlx_new_image(data.mlx, SCREEN_WIDTH,
			SCREEN_HEIGHT);
	mlx_hook(data.win, DestroyNotify, 0, destroy_close, &data.mlx);
	mlx_hook(data.win, KeyPress, KeyPressMask, (int (*)())on_key_pressed,
		&data);
	// mlx_key_hook(data.win, (int (*)(void *))on_key_pressed, &data);
	mlx_loop_hook(data.mlx, (int (*)(void *))loop, &data);
	mlx_loop(data.mlx);
	mlx_destroy_image(data.mlx, data.rendering_buffer);
	mlx_destroy_window(data.mlx, data.win);
	mlx_destroy_display(data.mlx);
	free(data.mlx);
	return (0);
}

// char map[MAP_HEIGHT * MAP_WIDTH] = {
// 	1, 1, 1, 1, 1,
// 	1, 0, 0, 0, 1,
// 	1, 0, 0, 0, 1,
// 	1, 0, 0, 0, 1,
// 	1, 1, 1, 1, 1,
// };

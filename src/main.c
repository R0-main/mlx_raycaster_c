/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rguigneb <rguigneb@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/28 10:15:20 by rguigneb          #+#    #+#             */
/*   Updated: 2025/04/11 16:13:24 by rguigneb         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "mlx.h"
#include "mlx_int.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#define PI 3.1415926535

#define MAP_HEIGHT 7
#define MAP_WIDTH 10
#define SIZE 64
#define FOV 60 * (PI / 180)
#define MAX_ITERATION 10

#define SCREEN_HEIGHT MAP_HEIGHT *SIZE
#define SCREEN_WIDTH MAP_WIDTH *SIZE

#define X_SIZE 4
#define RAYS_COUNT (int)((SCREEN_WIDTH) / X_SIZE)

#define PLAYER_SPEED 1

typedef struct s_uvec_2
{
	unsigned int	x;
	unsigned int	y;
}					t_uvec2;

typedef struct s_vec_2
{
	int				x;
	int				y;
}					t_vec_2;

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
		data->player.position.x += adj;
		data->player.position.y += opo;
	}
	if (key == 's')
	{
		adj = cos(data->player.rotation_angle) * 3.5;
		opo = sin(data->player.rotation_angle) * 3.5;
		data->player.position.x -= adj;
		data->player.position.y -= opo;
	}
	if (key == 'a')
	{
		data->player.rotation_angle -= 0.05;
	}
	if (key == 'd')
	{
		data->player.rotation_angle += 0.05;
	}
	data->player.rotation_angle = normalize_angle(data->player.rotation_angle);
}

int	get_distance_from_the_camera(t_data *data)
{
	float	half_screen;
	float	half_fov;

	half_screen = SCREEN_WIDTH / 2;
	half_fov = FOV / 2;
	return (half_screen / tan(half_fov));
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
	if (pos.x < 0)
		pos.x = 0;
	if (pos.x > SCREEN_WIDTH)
		pos.x = SCREEN_WIDTH;
	if (pos.y < 0)
		pos.y = 0;
	if (pos.y > SCREEN_HEIGHT)
		pos.y = SCREEN_HEIGHT;
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
	if (end.y < 0 && abs(end.y - start.y) > abs(end.x - start.x))
	{
		tmp = start;
		start = end;
		end = tmp;
	}
	distance_x = end.x - start.x;
	distance_y = end.y - start.y;
	step = fmaxf(fabsf(distance_x), fabsf(distance_y));
	if (step != 0)
	{
		stepX = distance_x / step;
		stepY = distance_y / step;
	}
	while (i <= step + 1)
	{
		y = roundf(start.y + i * stepY);
		x = roundf(start.x + i * stepX);
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

void	draw_straight_line(t_img *buffer, int color, int x, int height)
{
	int y;
	int t = (SCREEN_HEIGHT - height) / 2;

	y = t;
	while (y < SCREEN_HEIGHT - t)
	{
		put_pixel_to_buffer(buffer, (t_uvec2){x, y}, color);
		y++;
	}
	y = t;
	while (y < SCREEN_HEIGHT - t)
	{
		put_pixel_to_buffer(buffer, (t_uvec2){x + 1, y}, color);
		y++;
	}
	y = t;
	while (y < SCREEN_HEIGHT - t)
	{
		put_pixel_to_buffer(buffer, (t_uvec2){x + 2, y}, color);
		y++;
	}
	y = t;
	while (y < SCREEN_HEIGHT - t)
	{
		put_pixel_to_buffer(buffer, (t_uvec2){x + 3, y}, color);
		y++;
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
			if (map[tmp.y * MAP_WIDTH + tmp.x] == 1)
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

bool	is_wall(char *map, t_vec_2 pos)
{
	if (pos.x < 0 || pos.x >= SCREEN_WIDTH || pos.y < 0
		|| pos.y >= SCREEN_HEIGHT)
		return (true);
	return (map[(pos.y / SIZE) * MAP_WIDTH + (pos.x / SIZE)] == 1);
}

bool	is_looking_bottom_right(float angle)
{
	return (angle >= 0 && angle <= PI / 2);
}

bool	is_looking_bottom_left(float angle)
{
	return (angle >= PI / 2 && angle <= PI);
}

bool	is_looking_top(float angle)
{
	return (angle <= PI && angle >= 0);
}

bool	is_looking_left(float angle)
{
	return (angle <= PI + (PI / 2) && angle >= PI / 2);
}

bool	is_looking_right(float angle)
{
	return (angle >= 0 && angle <= PI / 2 || angle >= PI + (PI / 2)
		&& angle <= 2 * PI);
}

bool	is_looking_bottom(float angle)
{
	return (angle >= PI && angle <= PI * 2);
}

bool	is_looking_top_left(float angle)
{
	return (angle >= PI && angle <= PI + PI / 2);
}

bool	is_looking_top_right(float angle)
{
	return (angle >= PI + PI / 2 && angle <= 2 * PI);
}

t_uvec2	get_horizontal_colision(t_img *buffer, t_fvec2 start, char *map,
		float angle)
{
	t_uvec2	dist;
	int		i;
	t_vec_2	A;
	t_vec_2	pos_to_check;
	int		Ya;

	i = 0;
	dist = (t_uvec2){0, 0};
	if (is_looking_bottom(angle))
		A.y = ((int)start.y / SIZE) * SIZE - 1;
	else
		A.y = ((int)start.y / SIZE) * SIZE + SIZE;
	Ya = 0;
	if (is_looking_bottom(angle))
		Ya = -SIZE;
	else
		Ya = SIZE;
	A.x = ((A.y - start.y) / tan(angle)) + start.x;
	while (i < MAX_ITERATION && A.y / SIZE >= 0 && A.y / SIZE <= MAP_HEIGHT)
	{
		dist.x = A.x - start.x;
		dist.y = A.y - start.y;
		if (is_wall(map, (t_vec_2){A.x, A.y}))
			break ;
		A.x += Ya / tan(angle);
		A.y += Ya;
		i++;
	}
	dist.x += start.x;
	dist.y += start.y;
	return ((t_uvec2){A.x, A.y});
}

t_uvec2	get_vertical_colision(t_img *buffer, t_fvec2 start, char *map,
		float angle)
{
	t_uvec2	dist;
	int		i;
	t_vec_2	B;
	int		Xa;
	int		Ya;

	i = 0;
	dist = (t_uvec2){start.x, start.y};
	if (is_looking_left(angle))
		B.x = ((int)start.x / SIZE) * SIZE - 1;
	else
		B.x = ((int)start.x / SIZE) * SIZE + SIZE;
	if (is_looking_left(angle))
		Xa = -SIZE;
	else
		Xa = SIZE;
	Ya = Xa * tan(angle);
	B.y = ((int)start.y) + ((int)start.x - B.x) * -tan(angle);
	while (i < MAX_ITERATION && B.x / SIZE >= 0 && B.x / SIZE <= MAP_WIDTH)
	{
		dist.x = B.x - start.x;
		dist.y = B.y - start.y;
		if (is_wall(map, B))
			break ;
		B.x += Xa;
		B.y += Ya;
		i++;
	}
	dist.x += start.x;
	dist.y += start.y;
	return (dist);
}

// t_uvec2	get_horizontal_colision(t_fvec2 start, char *map, float angle)
// {
// 	t_vec_2 A;
// 	t_uvec2 dist;
// 	int i;

// 	i = 0;
// 	dist = (t_uvec2){start.x, start.y};
// 	if (angle >= PI && angle <= 2 * PI)
// 		A.y = ((int)start.y / SIZE);
// 	else
// 		A.y = ((int)start.y / SIZE) + 1;
// 	while (i < MAX_ITERATION && A.y >= 0 && A.y <= MAP_HEIGHT)
// 	{
// 		A.x = (((A.y * SIZE) - start.y) / tan(angle))
// 			+ start.x;
// 		if (angle >= PI && angle <= 2 * PI && map[(A.y - 1) * MAP_HEIGHT
// 			+ (A.x / SIZE)] == 1)
// 			i = MAX_ITERATION;
// 		else if (map[(A.y) * MAP_HEIGHT + (A.x / SIZE)] == 1)
// 			i = MAX_ITERATION;
// 		dist.x = A.x - start.x;
// 		dist.y = (A.y * SIZE) - start.y;
// 		if (angle >= PI && angle <= 2 * PI)
// 			A.y--;
// 		else
// 			A.y++;
// 		i++;
// 	}
// 	dist.x += start.x;
// 	dist.y += start.y;
// 	return (dist);
// }

// t_uvec2	get_vertical_colision(t_fvec2 start, char *map, float angle)
// {
// 	t_vec_2 A;
// 	t_uvec2 dist;
// 	int i;

// 	i = 0;
// 	dist = (t_uvec2){start.x, start.y};
// 	if (angle >= PI / 2 && angle <= PI + 2 * PI)
// 		A.x = ((int)start.x / SIZE);
// 	else
// 		A.x = ((int)start.x / SIZE) + 1;
// 	while (i < MAX_ITERATION && A.x >= 0 && A.x <= MAP_WIDTH)
// 	{
// 		A.y = (int)start.y + ((int)start.x - (A.x * SIZE)) * tan(angle);
// 		// if (angle >= PI / 2 && angle <= PI + 2 * PI && map[(A.y) * MAP_HEIGHT
// 		// 	+ ((A.x) * SIZE)] == 1)
// 		// 	i = MAX_ITERATION;
// 		// else if (map[(A.y) * MAP_HEIGHT + (A.x * SIZE)] == 1)
// 		// 	i = MAX_ITERATION;
// 		dist.x = (A.x * SIZE) - start.x;
// 		dist.y = A.y - start.y;
// 		if (angle >= PI / 2 && angle <= PI + 2 * PI)
// 			A.x++;
// 		else
// 			A.x--;
// 		i++;
// 	}
// 	dist.x += start.x;
// 	dist.y += start.y;
// 	return (dist);
// }

int	distance_between(t_uvec2 vec1, t_uvec2 vec2)
{
	int	dx;
	int	dy;

	dx = (int)vec2.x - (int)vec1.x;
	dy = (int)vec2.y - (int)vec1.y;
	return (int)(sqrt(dx * dx + dy * dy));
}

void	draw_ray(t_img *buffer, t_player player, t_data *data, float angle,
		int i)
{
	t_uvec2	vert;
	t_uvec2	hori;
	t_uvec2	pos;
	float	height;
	float	start;
	float	distance;
	float	r;

	// ((unsigned int *)buffer->data)[y * (buffer->size_line / 4) + x]
	// int	Bx;
	// int	By;
	// if (angle >= PI / 2 && angle <= PI + PI / 2)
	// 	Bx = (player.position.x / SIZE);
	// else
	// 	Bx = (player.position.x / SIZE) + 1;
	// By = ((int)player.position.y) + ((int)player.position.x - (Bx * SIZE))
	// * tan(angle);
	// distX = (Bx * SIZE) - player.position.x;
	// distY = By - player.position.y;
	vert = get_vertical_colision(data->rendering_buffer, player.position,
			data->map, angle);
	hori = get_horizontal_colision(data->rendering_buffer, player.position,
			data->map, angle);
	// draw_rect(buffer, 0xDDDD00, (t_uvec2){vert.x - 2, vert.y - 2},
	// 	(t_uvec2){vert.x + 2, vert.y + 2});
	// draw_rect(buffer, 0x0000DD, (t_uvec2){hori.x - 2, hori.y - 2},
	// 	(t_uvec2){hori.x + 2, hori.y + 2});
	if (distance_between((t_uvec2){(int)player.position.x,
			(int)player.position.y},
			hori) < distance_between((t_uvec2){(int)player.position.x,
			(int)player.position.y}, vert))
		pos = hori;
	else
		pos = vert;
	// printf("dist from hor %d\n",
	// 	distance_between((t_uvec2){(int)player.position.x,
	// 		(int)player.position.y}, hori));
	// printf("dist from vert %d\n",
	// 	distance_between((t_uvec2){(int)player.position.x,
	// 		(int)player.position.y}, vert));
	// printf("choosen : %d %d\n", pos.x, pos.y);
	// draw_line(buffer, 0x00DD00, (t_uvec2){(int)player.position.x,
	// 	(int)player.position.y}, (t_uvec2)hori);
	// draw_line(buffer, 0x0000DD, (t_uvec2){(int)player.position.x,
	// 	(int)player.position.y}, (t_uvec2)vert);
	distance = distance_between((t_uvec2){(int)player.position.x,
			(int)player.position.y}, pos);
	r = get_distance_from_the_camera(data);
	// draw_line(buffer, 0xFF0000, (t_uvec2){(int)player.position.x,
	// 	(int)player.position.y}, (t_uvec2)pos);
	height = (32 / distance) * r;
	start = (SCREEN_HEIGHT / 2) - (height / 2);
	draw_straight_line(buffer, 0x00DD00, i, height);
	// draw_line(buffer, 0x0DD000, (t_uvec2){player.position.x,
	// player.position.y},
	// 	(t_uvec2){player.position.x + cos(angle) * len, player.position.y
	// 	+ sin(angle) * len});
}

void	raycaster(t_img *buffer, t_player player, float angle, int len)
{
	draw_line(buffer, 0xFF00FF, (t_uvec2){player.position.x, player.position.y},
		(t_uvec2){player.position.x + cos(angle) * len, player.position.y
		+ sin(angle) * len});
}

void	draw_player(t_img *buffer, t_data *data, t_player player)
{
	int		i;
	float	angle;
	int		x;

	// draw_rect(buffer, 0xFF0000, (t_uvec2){((int)player.position.x) - 5,
	// 	((int)player.position.y) - 5}, (t_uvec2){((int)player.position.x) + 5,
	// 	((int)player.position.y) + 5});
	// raycaster(buffer, player, player.rotation_angle, 15);
	// // printf("rotaton %02f\n", player.rotation_angle * PI * 180);
	// draw_ray(buffer, player, data, player.rotation_angle, SCREEN_WIDTH / 2);
	i = 0;
	x = 0;
	angle = player.rotation_angle - (FOV / 2);
	while (i < RAYS_COUNT)
	{
		draw_ray(buffer, player, data, normalize_angle(angle), x);
		i += 1;
		x += X_SIZE;
		angle += FOV / RAYS_COUNT;
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
	// printf("distance : %d\n", get_distance_from_the_camera(data));
	data->rendering_buffer = mlx_new_image(data->mlx, SCREEN_WIDTH,
			SCREEN_HEIGHT);
	// draw_line(data->rendering_buffer, 0xFF0000, (t_uvec2){50, 200},
	// (t_uvec2){0,
	// 	0});
	// draw_line(data->rendering_buffer, 0xDD00DD, (t_uvec2){0, 0},
	// (t_uvec2){50,
	// 	90});
	// draw_rect(data->rendering_buffer, 0x00DF0F, (t_uvec2){90, 90},
	// 	(t_uvec2){120, 99});
	// draw_map(data->rendering_buffer, data->map);
	draw_player(data->rendering_buffer, data, data->player);
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
	char	map[MAP_HEIGHT * MAP_WIDTH] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
			0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0,
			0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0,
			0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};

	bzero(&data, sizeof(t_data));
	data.player.position.x = SCREEN_WIDTH / 2 - SIZE;
	data.player.position.y = SCREEN_HEIGHT / 2 - SIZE;
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

// char map[MAP_HEIGHT * MAP_WIDTH] = {
// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
// 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
// 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
// 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
// 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
// 1, 0, 0, 0, 0, 0, 0, 0, 0, 1,
// 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
// };

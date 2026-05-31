import matplotlib.pyplot as plt
import mpmath as mp
import numpy as np
import sympy as sp


# Вычисляем значение показателя преломления среды внутри волновода
def calculate_n2_value(coords):
    x = coords[0]
    y = coords[1]
    z = coords[2]
    return 3 - sp.exp(-0.1 * z) + 0.1 * sp.sin(x) + 0.2 * sp.sin(0.5 * y ** 2)
    # return 1


# Ищем частные производные n2 по всем координатам - градиент n2
def calculate_grad_n2_formula():
    x, y, z = sp.symbols('x y z')
    n2 = 3 - sp.exp(-0.1 * z) + 0.1 * sp.sin(x) + 0.2 * sp.sin(0.5 * y ** 2)
    return [sp.diff(n2, var) for var in (x, y, z)]
    # return [0 * x, 0 * y, 0 * z]


# Вычисляем изменение направления луча (приращение de касательной к лучу)
def calculate_de_value(coords, e_coords):
    x = coords[0]
    y = coords[1]
    z = coords[2]

    # значение плотности среды n2 в данной точке внутри волновода
    n2_value = calculate_n2_value(coords)
    # частные производные dn2 по dl считаем как delta/dl
    # delta - разность значений n2 между текущей координатой и предыдущей
    delta_x_n2 = calculate_n2_value(np.array([x + dl, y, z])) - n2_value
    delta_y_n2 = calculate_n2_value(np.array([x, y + dl, z])) - n2_value
    delta_z_n2 = calculate_n2_value(np.array([x, y, z + dl])) - n2_value
    delta = np.array([delta_x_n2, delta_y_n2, delta_z_n2])

    # ∇n2 = градиент n2
    grad_x, grad_y, grad_z = calculate_grad_n2_formula()
    grad_n2_value = np.array(
        [grad_x.subs(sp.Symbol('x'), x), grad_y.subs(sp.Symbol('y'), y), grad_z.subs(sp.Symbol('z'), z)])

    # de
    return 1 / n2_value * (grad_n2_value - delta / dl * e_coords) * dl


# Вычисление отраженного луча
def reflect(coords, axis):
    # нормализуем оба вектора
    coords = coords / mp.hypot(mp.hypot(coords[0], coords[1]), coords[2])
    axis = axis / mp.hypot((mp.hypot(axis[0], axis[1])), axis[2])
    # проекция луча на ось в направлении оси
    projection = np.dot(coords, axis)
    # отражение луча в плоскости, перпендикулярной оси
    coords = coords - 2 * projection * axis
    return coords


# Вычисление длины луча
def calculate_eikonal_length():
    # сферические координаты e0, необходимые для задания начального направления хода луча
    e0_x = dl * sp.cos(phi_angle) * sp.sin(theta_angle)
    e0_y = dl * sp.sin(phi_angle) * sp.sin(theta_angle)
    e0_z = dl * sp.cos(phi_angle)
    # координаты вектора касательной к лучу, e = e0
    e_coords = np.array([e0_x, e0_y, e0_z])
    # начальные координаты, r = r0
    coords = np.array([x0, y0, z0])

    # добавляем начальные координаты в массив
    x_coords.append(x0)
    y_coords.append(y0)
    z_coords.append(z0)

    # длина луча
    length = 0
    # пока не прошли весь волновод вдоль
    while coords[2] < L:
        # e = e + de
        e_coords += calculate_de_value(coords, e_coords)
        # r = r + e * dl
        new_coords = coords + e_coords * dl

        # если луч пересекает стенку волновода
        if new_coords[0] ** 2 + new_coords[1] ** 2 >= R ** 2:
            # считаем нормаль к поверхности в данной точке волновода
            normal = - np.array([new_coords[0], new_coords[1], 0]) / mp.hypot(new_coords[0], new_coords[1])
            # отражаем вектор e
            e_coords = reflect(e_coords, normal)
            continue

        # часть длины луча - декартово расстояние между точками new_coords и coords
        length += mp.hypot(mp.hypot(new_coords[0] - coords[0], new_coords[1] - coords[1]), new_coords[2] - coords[2])
        # обновили координату r
        coords = new_coords

        # добавляем координаты в массив
        x_coords.append(coords[0])
        y_coords.append(coords[1])
        z_coords.append(coords[2])

    # возвращаем длину луча
    return length


# Вывод графика
def draw_plot():
    ax = plt.figure().add_subplot(111, projection='3d')
    z = np.linspace(0, L, 2)
    angle = np.linspace(0, 2 * np.pi, 50)
    angle_grid, z_grid = np.meshgrid(angle, z)
    x_grid = R * np.cos(angle_grid)
    y_grid = R * np.sin(angle_grid)
    ax.plot_surface(x_grid, y_grid, z_grid, color='plum', alpha=0.5)
    ax.plot(x_coords, y_coords, z_coords, color='black')
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    plt.show()


# Дано
# n2 тоже задано
R = 2.5  # радиус волновода
L = 30  # длина волновода
x0 = 0  # начальная координата X
y0 = 1.6666667  # начальная координата Y
z0 = 0  # начальная координата Z
phi_angle = mp.radians(40)  # угол с положительной осью Z
theta_angle = mp.radians(20)  # угол с положительной осью Х
n1 = 1  # показатель преломления среды вне волновода

# Задаю сама
dl = 0.001  # шаг
grad_n2_formula = calculate_grad_n2_formula()  # градиент n2
# все координаты
x_coords = []
y_coords = []
z_coords = []

if __name__ == '__main__':
    result = calculate_eikonal_length()
    draw_plot()
    print("Длина луча:", result)

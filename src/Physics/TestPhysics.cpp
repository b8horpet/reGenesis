/*
#author: b8horpet

import unittest

import sys,os,inspect
sys.path.append((os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe()))))+"/..")
from Physics import *


class TestBasics(unittest.TestCase):
	def test_Vector2D_add(self):
		a=Vector2D(1,2)
		b=Vector2D(3,4)
		c=a+b
		self.assertEqual(c.x,4)
		self.assertEqual(c.y,6)
		b-=a
		self.assertEqual(b.x,2)
		self.assertEqual(b.y,2)

	def test_Vector2D_dot(self):
		a=Vector2D(2,3)
		b=Vector2D(4,2)
		c=a*b
		self.assertEqual(c,14)

	def test_Vector2D_scalar(self):
		a=Vector2D(2,3)
		b=5*a
		self.assertEqual(b.x,10)
		self.assertEqual(b.y,15)
		a*=2
		self.assertEqual(a.x,4)
		self.assertEqual(a.y,6)

	def test_Vector2D_div(self):
		a=Vector2D(4,8)
		b=a/2
		self.assertEqual(b.x,2)
		self.assertEqual(b.y,4)
		a/=4
		self.assertEqual(a.x,1)
		self.assertEqual(a.y,2)
		with self.assertRaises(NotImplementedError):
			c=1/a

	def test_Vector2D_len(self):
		a=Vector2D(3,4)
		l=abs(a)
		self.assertAlmostEqual(l,5)

	def test_Vector3D_add(self):
		a=Vector3D(1,2,3)
		b=Vector3D(4,5,6)
		c=a+b
		self.assertEqual(c.x,5)
		self.assertEqual(c.y,7)
		self.assertEqual(c.z,9)
		b-=a
		self.assertEqual(b.x,3)
		self.assertEqual(b.y,3)
		self.assertEqual(b.z,3)

	def test_Vector3D_dot(self):
		a=Vector3D(2,3,5)
		b=Vector3D(4,2,3)
		c=a*b
		self.assertEqual(c,29)

	def test_Vector3D_scalar(self):
		a=Vector3D(2,3,4)
		b=3*a
		self.assertEqual(b.x,6)
		self.assertEqual(b.y,9)
		self.assertEqual(b.z,12)
		a*=5
		self.assertEqual(a.x,10)
		self.assertEqual(a.y,15)
		self.assertEqual(a.z,20)

	def test_Vector3D_div(self):
		a=Vector3D(4,8,16)
		b=a/2
		self.assertEqual(b.x,2)
		self.assertEqual(b.y,4)
		self.assertEqual(b.z,8)
		a/=4
		self.assertEqual(a.x,1)
		self.assertEqual(a.y,2)
		self.assertEqual(a.z,4)
		with self.assertRaises(NotImplementedError):
			c=1/a

	def test_Vector3D_len(self):
		a=Vector3D(3,4,12)
		l=abs(a)
		self.assertAlmostEqual(l,13)

	def test_Vector3D_cross(self):
		a=Vector3D(2,1,-1)
		b=Vector3D(1,3,5)
		c=a%b
		self.assertEqual(c.x,8)
		self.assertEqual(c.y,-11)
		self.assertEqual(c.z,5)


class TestPhysics(unittest.TestCase):
	class mockrandom:
		pass
	def test_CreateWorld(self):
		w=World(TestPhysics.mockrandom())
		w.Objects.append(Sphere())
		w.Creatures.append(Creature())

if __name__ == '__main__':
	unittest.main()
*/
#!/usr/bin/env python3
"""
generer_icones.py
=================
Script pour générer les icônes PNG 32x32 de TurboNœuds

Usage: python generer_icones.py
"""

from PIL import Image, ImageDraw
import os

def create_point_icon():
    """Crée l'icône du point (bleu)"""
    img = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.ellipse([8, 8, 24, 24], fill=(0, 100, 255, 255))
    img.save('point.png')
    print("✓ point.png créé")

def create_repere_icon():
    """Crée l'icône du repère (losange jaune)"""
    img = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    points = [(16, 4), (28, 16), (16, 28), (4, 16)]
    draw.polygon(points, fill=(255, 200, 0, 255), outline=(200, 150, 0, 255))
    img.save('repere.png')
    print("✓ repere.png créé")

def create_entrelacs_icon():
    """Crée l'icône de l'entrelacs (vert)"""
    img = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.arc([4, 4, 28, 28], 0, 180, fill=(0, 180, 0, 255), width=3)
    draw.arc([4, 8, 28, 32], 180, 360, fill=(0, 180, 0, 255), width=3)
    img.save('entrelacs.png')
    print("✓ entrelacs.png créé")

def create_gomme_icon():
    """Crée l'icône de la gomme (rose)"""
    img = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    points = [(8, 12), (20, 12), (24, 20), (4, 20)]
    draw.polygon(points, fill=(255, 150, 180, 255), outline=(200, 100, 130, 255))
    img.save('gomme.png')
    print("✓ gomme.png créé")

def create_fleche_icon():
    """Crée l'icône de la flèche bidirectionnelle (gris)"""
    img = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.polygon([(4, 16), (12, 10), (12, 14)], fill=(100, 100, 100, 255))
    draw.rectangle([12, 14, 20, 18], fill=(100, 100, 100, 255))
    draw.polygon([(28, 16), (20, 10), (20, 14)], fill=(100, 100, 100, 255))
    img.save('fleche.png')
    print("✓ fleche.png créé")

def create_cylindre_icon():
    """Crée l'icône du cylindre (bleu clair)"""
    img = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.ellipse([8, 4, 24, 12], fill=(100, 150, 255, 255), outline=(50, 100, 200, 255))
    draw.rectangle([8, 8, 24, 24], fill=(100, 150, 255, 255))
    draw.ellipse([8, 20, 24, 28], fill=(100, 150, 255, 255), outline=(50, 100, 200, 255))
    img.save('Cylindre.png')
    print("✓ Cylindre.png créé")

def create_croix_icon():
    """Crée l'icône de la croix (violet)"""
    img = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.rectangle([14, 6, 18, 26], fill=(150, 50, 200, 255))
    draw.rectangle([6, 14, 26, 18], fill=(150, 50, 200, 255))
    img.save('croix.png')
    print("✓ croix.png créé")

def create_rotation_icon():
    """Crée l'icône de rotation (vert foncé)"""
    img = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.arc([6, 6, 26, 26], 45, 315, fill=(50, 150, 50, 255), width=3)
    draw.polygon([(26, 8), (22, 4), (22, 12)], fill=(50, 150, 50, 255))
    img.save('rotation.png')
    print("✓ rotation.png créé")

def create_bascule_icon():
    """Crée l'icône de bascule (orange)"""
    img = Image.new('RGBA', (32, 32), (0, 0, 0, 0))
    draw = ImageDraw.Draw(img)
    draw.polygon([(16, 8), (10, 14), (14, 14)], fill=(200, 100, 0, 255))
    draw.polygon([(16, 24), (22, 18), (18, 18)], fill=(200, 100, 0, 255))
    img.save('bascule.png')
    print("✓ bascule.png créé")

def main():
    """Fonction principale"""
    print("=" * 50)
    print("Génération des icônes TurboNœuds")
    print("=" * 50)
    print()
    
    try:
        create_point_icon()
        create_repere_icon()
        create_entrelacs_icon()
        create_gomme_icon()
        create_fleche_icon()
        create_cylindre_icon()
        create_croix_icon()
        create_rotation_icon()
        create_bascule_icon()
        
        print()
        print("=" * 50)
        print("✓ Toutes les icônes ont été générées avec succès !")
        print("=" * 50)
        
    except Exception as e:
        print(f"\n❌ Erreur lors de la génération : {e}")
        print("\nAssurez-vous que Pillow est installé :")
        print("  pip install Pillow")

if __name__ == "__main__":
    main()

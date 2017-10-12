# Resume Generated with PDFML

The resume in this folder is generated with the command:
```
nick template.pdfml main.pdfml -o FelixGuoResume.pdf
```
As you can see, files can be seperated and will be compiled one by one in order.
Since they are compiled in order, the files must be specified in order of dependency. The resume in this folder is a remake of my actual resume (created with Adobe Indesign) at https://felixguo.me/FelixGuoResume.pdf. As you can see, with only `LinearLayout`s and `TextView`s as well as a well built templating system, most documents can be easily written in PDFML. 